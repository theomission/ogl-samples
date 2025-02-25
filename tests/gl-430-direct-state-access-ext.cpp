///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-430/direct-state-access.vert");
	char const * FRAG_SHADER_SOURCE("gl-430/direct-state-access.frag");
	char const * TEXTURE_DIFFUSE("kueken7_rgba8_srgb.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(80, 60);

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2,
		2, 3, 0
	};

	namespace program
	{
		enum type
		{
			VERTEX,
			FRAGMENT,
			MAX
		};
	}//namespace program

	namespace framebuffer
	{
		enum type
		{
			RENDER,
			RESOLVE,
			MAX
		};
	}//namespace framebuffer

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			TEXTURE,
			MULTISAMPLE,
			COLORBUFFER,
			MAX
		};
	}//namespace texture
}//namespace

class instance : public test
{
public:
	instance(int argc, char* argv[]) :
		test(argc, argv, "gl-430-direct-state-access-ext", test::CORE, 4, 3, glm::uvec2(640, 480), glm::vec2(glm::pi<float>() * 0.1f)),
		VertexArrayName(0),
		PipelineName(0),
		ProgramName(0),
		SamplerName(0),
		UniformBlockSize(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, texture::MAX> TextureName;
	std::array<GLuint, framebuffer::MAX> FramebufferName;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint SamplerName;
	GLint UniformBlockSize;

	bool initProgram()
	{
		bool Validated = true;

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 430 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 430 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glGenBuffers(buffer::MAX, &BufferName[0]);
		glNamedBufferDataEXT(BufferName[buffer::ELEMENT], ElementSize, ElementData, GL_STATIC_DRAW);
		glNamedBufferDataEXT(BufferName[buffer::VERTEX], VertexSize, VertexData, GL_STATIC_DRAW);
		glNamedBufferDataEXT(BufferName[buffer::TRANSFORM], UniformBlockSize, nullptr, GL_DYNAMIC_DRAW);

		return true;
	}

	bool initSampler()
	{
		glGenSamplers(1, &SamplerName);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterfv(SamplerName, GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		return true;
	}

	bool initTexture()
	{
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		gli::gl GL;
		gli::gl::format const Format = GL.translate(Texture.format());
		gli::gl::swizzles const Swizzles = GL.translate(Texture.swizzles());

		glGenTextures(texture::MAX, &TextureName[0]);

		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, Swizzles[0]);
		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, Swizzles[1]);
		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, Swizzles[2]);
		glTextureParameteriEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, Swizzles[3]);

		glTextureStorage2DEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D, GLint(Texture.levels()), Format.Internal, GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTextureSubImage2DEXT(TextureName[texture::TEXTURE], GL_TEXTURE_2D,
				GLint(Level),
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), GLsizei(Texture[Level].dimensions().y),
				Format.External, Format.Type,
				Texture[Level].data());
		}

		glTextureParameteriEXT(TextureName[texture::MULTISAMPLE], GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteriEXT(TextureName[texture::MULTISAMPLE], GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
		glTextureStorage2DMultisampleEXT(TextureName[texture::MULTISAMPLE], GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_FALSE);

		glTextureParameteriEXT(TextureName[texture::COLORBUFFER], GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteriEXT(TextureName[texture::COLORBUFFER], GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTextureStorage2DEXT(TextureName[texture::COLORBUFFER], GL_TEXTURE_2D, 1, GL_RGBA8, GLsizei(FRAMEBUFFER_SIZE.x), GLsizei(FRAMEBUFFER_SIZE.y));

		return true;
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glNamedFramebufferTextureEXT(FramebufferName[framebuffer::RENDER], GL_COLOR_ATTACHMENT0, TextureName[texture::MULTISAMPLE], 0);
		glNamedFramebufferTextureEXT(FramebufferName[framebuffer::RESOLVE], GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);

		if(glCheckNamedFramebufferStatusEXT(FramebufferName[framebuffer::RENDER], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		if(glCheckNamedFramebufferStatusEXT(FramebufferName[framebuffer::RESOLVE], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), 0);
		glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), sizeof(glm::vec2));
		glEnableVertexArrayAttribEXT(VertexArrayName, semantic::attr::POSITION);
		glEnableVertexArrayAttribEXT(VertexArrayName, semantic::attr::TEXCOORD);

		return true;
	}

	bool begin()
	{
		bool Validated = this->checkExtension("GL_EXT_direct_state_access");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();

		//glEnable(GL_SAMPLE_MASK);
		//glSampleMaski(0, 0xFF);

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteSamplers(1, &SamplerName);

		return true;
	}

	void renderFBO()
	{
		//glEnable(GL_SAMPLE_MASK);
		//glSampleMaski(0, 0xFF);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(4.0f);

		glViewportIndexedf(0, 0, 0, static_cast<float>(FRAMEBUFFER_SIZE.x), static_cast<float>(FRAMEBUFFER_SIZE.y));
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], 0, this->UniformBlockSize);
		glBindSampler(0, SamplerName);
		glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, TextureName[texture::TEXTURE]);
		glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0, 0);

		glDisable(GL_MULTISAMPLE);
	}

	bool render()
	{
		{
			glm::uint8* Pointer = reinterpret_cast<glm::uint8*>(glMapNamedBufferRangeEXT(BufferName[buffer::TRANSFORM],
				0, this->UniformBlockSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / FRAMEBUFFER_SIZE.y, 0.1f, 100.0f);
			*reinterpret_cast<glm::mat4*>(Pointer + 0) = Projection * this->view() * glm::mat4(1);

			// Make sure the uniform buffer is uploaded
			glUnmapNamedBufferEXT(BufferName[buffer::TRANSFORM]);
		}

		glBindProgramPipeline(PipelineName);

		// Step 1: render the scene in a multisampled framebuffer
		renderFBO();

		// Step 2: resolve MSAA
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferName[framebuffer::RESOLVE]);
		glBlitFramebuffer(
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y,
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// Step 3: Blit resolved colorbuffer. Resolve and blit can't be done in a single step
		glm::ivec2 const WindowSize(this->getWindowSize());

		glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferName[framebuffer::RESOLVE]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y,
			0, 0, WindowSize.x, WindowSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	instance Test(argc, argv);
	Error += Test();

	return Error;
}

