#pragma once

#include "includeAll.h"
#include "WeightedBlendedOIT.h"
#include "Brush.h"

using namespace nvtoken;

class TopazSample : public NvSampleApp
{
public:

	TopazSample(NvPlatformContext* platform);
	~TopazSample();

	/* virtual functions */
	void initUI();
	void initRendering();
	void draw();
	void reshape(int32_t width, int32_t height);

	void configurationCallback(NvEGLConfiguration& config);

	void loadModel(std::string filename, GLuint orientation);

	void compileShaders(std::string name,
		const char* vertexShaderFilename,
		const char* fragmentShaderFilename,
		const char* geometryShaderFilename = nullptr);

	GLuint compileProgram(NvGLSLProgram::ShaderSourceItem* src, size_t count);

private:

	void initScene();
	void initCommandList();
	void initFramebuffers(int32_t width, int32_t height);

	void pushTokenParameters(NVTokenSequence& sequence, size_t& offset, std::string& stream, GLuint fbo, GLuint state);
	void setTokenBuffers(TopazGLModel* model, std::string& stream, bool cornerPoints = false);

	void updateCommandListState();

	void recalculatePositionOfFormular(size_t i, size_t j, size_t idx);

	void initBuffer(GLenum target, GLuint& buffer, GLuint64& buffer64,
		GLsizeiptr size, const GLvoid* data, bool ismutable = false);

	struct GLSLProgram;
	void drawModel(GLenum mode, GLSLProgram& program, TopazGLModel& model);

	/* check result of standart draw ( without command list ) */
	void renderStandartWeightedBlendedOIT();

	/* command list inited */
	bool isTokenInternalsInited;

	/* ARB_bindless_texture support */
	bool bindlessVboUbo;

	/* NV_uniform_buffer_unified_memory support */
	bool hwsupport;

	size_t numberOfFormulars;

	enum DrawMode
	{
		DRAW_WEIGHT_BLENDED_STANDARD,
		DRAW_WEIGHT_BLENDED_TOKEN_LIST
	};

	uint32_t drawMode;

	enum StippleMode
	{
		STIPPLE_MODE_DISABLED,
		STIPPLE_MODE_ENABLED
	};

	uint32_t stippleMode;

	enum States
	{
		STATE_TRANSPARENT,
		STATE_TRASPARENT_LINES,
		STATE_COMPOSITE,
		STATES_COUNT
	};

	struct CmdList
	{
		CmdList()
		{
		}

		/* state objects of base opaque draw */
		std::map<GLenum, GLuint> stateObjects;

		GLuint          tokenBuffer;
		GLuint          tokenCmdList;

		NVTokenSequence tokenSequence;
		NVTokenSequence tokenSequenceList;

		std::string     tokenData;
	} cmdlist;

	struct Textures
	{
		Textures() : sceneColor(0), sceneDepth(0), skybox(0)
		{
		}

		GLuint sceneColor,
			sceneDepth,
			skybox;
	} textures;

	struct GLSLProgram
	{
	private:
		GLuint program;

	public:
		GLuint getProgram()
		{
			return this->program;
		}
		void setProgram(GLuint id)
		{
			this->program = id;
		}
	};

	struct TexturesAddress64
	{
		TexturesAddress64() : sceneColor(0), sceneDepth(0), skybox(0)
		{
		}

		GLuint64 sceneColor,
			sceneDepth,
			skybox;
	} texturesAddress64;

	struct framebuffer
	{
		framebuffer() : scene(0)
		{
		}

		GLuint scene;
	} fbos;

	struct uniformBuffer
	{
		uniformBuffer() : sceneUbo(0), sceneUbo64(0), weightBlendedUbo(0), weightBlendedUbo64(0)
		{
		}

		GLuint   sceneUbo;
		GLuint64 sceneUbo64;

		GLuint	 weightBlendedUbo;
		GLuint64 weightBlendedUbo64;
	} ubos;

	struct SceneData
	{
		nv::matrix4f projection;
		nv::vec4f	 backgroundColor;
		GLuint64	 sceneDepthId64;
		float		 depthScale;
	} sceneData;

	struct ObjectData
	{
		nv::matrix4f modelView;
		nv::vec4f objectID;
		nv::vec4f objectColor;
		GLuint64  skybox;
		GLuint64  pattern;
		GLint	  usePattern;
		float	  opacity;
	};

	struct WeightBlendedData
	{
		GLuint64 colorTex0;
		GLuint64 colorTex1;
		GLuint64 depthTexture;
	} weightBlendedData;

	struct FullScreenRectangle
	{
		FullScreenRectangle() : vboFullScreen(0), vboFullScreen64(0)
		{
		}

		GLuint	 vboFullScreen;
		GLuint64 vboFullScreen64;
	} fullScreenRectangle;

	std::map<std::string, std::unique_ptr<GLSLProgram> > shaderPrograms;
	std::vector<std::unique_ptr<TopazGLModel> >  models;

	std::vector<std::unique_ptr<ObjectData> > objectData;
	std::vector<std::unique_ptr<ObjectData> > linesObjectData;

	std::unique_ptr<WeightedBlendedOIT> oit;
	std::unique_ptr<BrushStyles> brushStyle;

	nv::vec4f sceneBackgroundColor;
};