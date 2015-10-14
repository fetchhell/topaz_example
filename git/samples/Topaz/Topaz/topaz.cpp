#include "topaz.h"
#include <windows.h>

TopazSample::TopazSample(NvPlatformContext* platform) : NvSampleApp(platform, "Topaz Sample"), drawMode(DRAW_WEIGHT_BLENDED_TOKEN_LIST),
stippleMode(STIPPLE_MODE_DISABLED), isTokenInternalsInited(false), numberOfFormulars(8)
{
	oit = std::unique_ptr<WeightedBlendedOIT>(new WeightedBlendedOIT);
	brushStyle = std::unique_ptr<BrushStyles>(new BrushStyles);

	sceneBackgroundColor = nv::vec4f(0.2f, 0.2f, 0.2f, 0.0f);
	m_transformer->setTranslationVec(nv::vec3f(0.0f, -0.1f, -2.5f));
	forceLinkHack();
}

TopazSample::~TopazSample()
{
}

void TopazSample::configurationCallback(NvEGLConfiguration& config)
{
	config.depthBits = 24;
	config.stencilBits = 0;
	config.apiVer = NvGfxAPIVersionGL4_4();
}

void TopazSample::initUI()
{
	if (mTweakBar)
	{
		NvTweakEnum<uint32_t> enumVals[] =
		{
			{ "weight blended standard", DRAW_WEIGHT_BLENDED_STANDARD },
			{ "weight blended token list", DRAW_WEIGHT_BLENDED_TOKEN_LIST }
		};

		NvTweakEnum<uint32_t> stippleVals[] =
		{
			{ "enable", STIPPLE_MODE_ENABLED },
			{ "disable", STIPPLE_MODE_DISABLED },
		};

		mTweakBar->addPadding();

		/* draw mode : DRAW_STANDART, DRAW_TOKEN_LIST */
		mTweakBar->addEnum("Draw Mode:", drawMode, enumVals, TWEAKENUM_ARRAYSIZE(enumVals));

		/* weighted blended oit parameters */
		mTweakBar->addValue("Opacity:", oit->getOpacity(), 0.0f, 1.0f);
		mTweakBar->addValue("Weight Parameter:", oit->getWeightParameter(), 0.1f, 1.0f);

		/* enable/disable stipple pattern */
		mTweakBar->addEnum("Stipple:", stippleMode, stippleVals, TWEAKENUM_ARRAYSIZE(stippleVals));

		mTweakBar->syncValues();
	}
}

void TopazSample::initRendering()
{
	if (!GLEW_ARB_bindless_texture)
	{
		LOGI("This sample requires ARB_bindless_texture");
		exit(EXIT_FAILURE);
	}

	bindlessVboUbo = GLEW_NV_vertex_buffer_unified_memory && requireExtension("GL_NV_uniform_buffer_unified_memory", false);

	NvAssetLoaderAddSearchPath("Topaz/Topaz");

	if (!requireMinAPIVersion(NvGfxAPIVersionGL4_4(), true))
	{
		exit(EXIT_FAILURE);
	}

	std::string registerInclude("/common.h");

	int32_t length;
	std::string srcHeader = NvAssetLoaderRead("shaders/common.h", length);

	glNamedStringARB(GL_SHADER_INCLUDE_ARB, registerInclude.length(), registerInclude.c_str(), srcHeader.length(), srcHeader.c_str());

	compileShaders("weightBlended", "shaders/vertex.glsl", "shaders/fragmentBlendOIT.glsl");
	compileShaders("weightBlendedFinal", "shaders/vertexOIT.glsl", "shaders/fragmentFinalOIT.glsl");

	loadModel("models/background.obj", NO_BBOX);
	loadModel("models/way1.obj", BBOX_Z);
	loadModel("models/way2.obj", BBOX_Z);
	loadModel("models/way3.obj", NO_BBOX);

	for (size_t i = 0; i < numberOfFormulars; i++)
	{
		loadModel("models/formular1.obj", BBOX_Y);
	}

	textures.skybox = NvImage::UploadTextureFromDDSFile("textures/sky_cube.dds");
}

void TopazSample::reshape(int32_t width, int32_t height)
{
	initFramebuffers(width, height);
	oit->InitAccumulationRenderTargets(width, height);

	initScene();
	initCommandList();
}

void TopazSample::recalculatePositionOfFormular(size_t i, size_t j, size_t idx)
{
	nv::vec4f wayPosition = nv::vec4f(models[i]->getCorners()[j], 1.0f);
	nv::vec4f formularPosition = nv::vec4f(j & 1 ? models[models.size() - numberOfFormulars]->getCorners().front() : 
		models[models.size() - numberOfFormulars]->getCorners().back(), 1.0f);

	nv::vec4f wayPositionTransform = m_transformer->getModelViewMat() * wayPosition;
	nv::vec4f formularPositionTransform = formularPosition;

	nv::vec3f delta(wayPositionTransform - formularPositionTransform);

	nv::matrix4f translation;
	translation.set_translate(delta);

	objectData[idx]->usePattern = false;
	if (stippleMode == STIPPLE_MODE_ENABLED)
	{
		objectData[idx]->usePattern = true;
	}

	objectData[idx]->opacity = oit->getOpacity();
	objectData[idx]->modelView = translation;

	linesObjectData[idx]->opacity = oit->getOpacity();
	linesObjectData[idx]->modelView = translation;
}

void TopazSample::draw()
{
	sceneData.depthScale = oit->getWeightParameter();

	/* update scene ubo */
	glNamedBufferSubDataEXT(ubos.sceneUbo, 0, sizeof(SceneData), &sceneData);

	glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);

	glViewport(0, 0, m_width, m_height);

	glClearColor(sceneBackgroundColor.x, sceneBackgroundColor.y, sceneBackgroundColor.z, sceneBackgroundColor.w);
	glClearDepthf(1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (drawMode == DRAW_WEIGHT_BLENDED_STANDARD)
	{
		renderStandartWeightedBlendedOIT();
	}
	else if (drawMode == DRAW_WEIGHT_BLENDED_TOKEN_LIST)
	{
		for (size_t i = 0; i < models.size() - numberOfFormulars; i++)
		{
			objectData[i]->usePattern = false;
			if (stippleMode == STIPPLE_MODE_ENABLED)
			{
				objectData[i]->usePattern = true;
			}

			objectData[i]->opacity = oit->getOpacity();
			objectData[i]->modelView = m_transformer->getModelViewMat();
			glNamedBufferSubDataEXT(models[i]->getBufferID("ubo"), 0, sizeof(ObjectData), objectData[i].get());

			if (models[i]->cornerPointsExists())
			{
				linesObjectData[i]->opacity = oit->getOpacity();
				linesObjectData[i]->modelView = m_transformer->getModelViewMat();
				glNamedBufferSubDataEXT(models[i]->getCornerBufferID("ubo"), 0, sizeof(ObjectData), linesObjectData[i].get());
			}
		}

		size_t idx = models.size() - numberOfFormulars;
		for (size_t i = 1; i < models.size() - numberOfFormulars; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				if (idx == models.size())
				{
					break;
				}

				if (!models[i]->cornerPointsExists())
				{
					continue;
				}

				recalculatePositionOfFormular(i, j, idx);

				glNamedBufferSubDataEXT(models[idx]->getBufferID("ubo"), 0, sizeof(ObjectData), objectData[idx].get());
				glNamedBufferSubDataEXT(models[idx]->getCornerBufferID("ubo"), 0, sizeof(ObjectData), linesObjectData[idx].get());

				idx++;
			}
		}

		if (fabs(oit->getOpacity() - 1.0) < 0.001)
		{
			if (oit->getBlendDst() != GL_ZERO)
			{
				oit->setBlendDst(GL_ZERO);
				updateCommandListState();
			}
		}
		else
		{
			if (oit->getBlendDst() != GL_ONE)
			{
				oit->setBlendDst(GL_ONE);
				updateCommandListState();
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, oit->getFramebufferID());

		GLfloat clearColorZero[4] = { 0.0f };
		GLfloat clearColorOne[4] = { 1.0f };

		glClearBufferfv(GL_COLOR, 0, clearColorZero);
		glClearBufferfv(GL_COLOR, 1, clearColorOne);
		glClearBufferfv(GL_DEPTH, 0, clearColorOne);

		glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
		glCallCommandListNV(cmdlist.tokenCmdList);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos.scene);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

GLuint TopazSample::compileProgram(NvGLSLProgram::ShaderSourceItem* src, size_t count)
{
	const GLchar *const ptr[] = { "/" };
	GLuint program = glCreateProgram();

	for (size_t i = 0; i < count; i++)
	{
		GLuint shader = glCreateShader(src[i].type);

		const char* sourceItems[2];

		int sourceCount = 0;
		sourceItems[sourceCount++] = (src[i].src);

		glShaderSource(shader, sourceCount, sourceItems, 0);
		glCompileShaderIncludeARB(shader, 1, ptr, nullptr);

		glAttachShader(program, shader);
		glDeleteShader(shader);
	}

	glLinkProgram(program);

	// check if program linked
	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		GLint bufLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
		if (bufLength)
		{
			char* buf = new char[bufLength];
			if (buf)
			{
				glGetProgramInfoLog(program, bufLength, NULL, buf);
				LOGI("Could not link program:\n%s\n", buf);
				delete[] buf;
			}
		}
		glDeleteProgram(program);
		program = 0;
	}

	return program;
}

void TopazSample::compileShaders(std::string name,
	const char* vertexShaderFilename,
	const char* fragmentShaderFilename,
	const char* geometryShaderFilename)
{
	std::unique_ptr<GLSLProgram> program(new GLSLProgram);

	int32_t sourceLength;
	std::vector<NvGLSLProgram::ShaderSourceItem> sources;

	NvGLSLProgram::ShaderSourceItem vertexShader;
	vertexShader.type = GL_VERTEX_SHADER;
	vertexShader.src = NvAssetLoaderRead(vertexShaderFilename, sourceLength);

	sources.push_back(vertexShader);

	if (geometryShaderFilename != nullptr)
	{
		NvGLSLProgram::ShaderSourceItem geometryShader;
		geometryShader.type = GL_GEOMETRY_SHADER;
		geometryShader.src = NvAssetLoaderRead(geometryShaderFilename, sourceLength);

		sources.push_back(geometryShader);
	}

	NvGLSLProgram::ShaderSourceItem fragmentShader;
	fragmentShader.type = GL_FRAGMENT_SHADER;
	fragmentShader.src = NvAssetLoaderRead(fragmentShaderFilename, sourceLength);

	sources.push_back(fragmentShader);

	program->setProgram(compileProgram(sources.data(), sources.size()));

	shaderPrograms[name] = std::move(program);
}

void TopazSample::loadModel(std::string filename, GLuint orientation)
{
	int32_t length;
	char* data = NvAssetLoaderRead(filename.c_str(), length);

	std::unique_ptr<TopazGLModel> model(new TopazGLModel());
	model->loadModelFromObjData(data);

	if (orientation != NO_BBOX)
	{
		model->calculateCornerPoints(1.0f, orientation);
	}

	model->rescaleModel(1.0f);

	models.push_back(std::move(model));

	NvAssetLoaderFree(data);
}

void TopazSample::initBuffer(GLenum target, GLuint& buffer, GLuint64& buffer64,
	GLsizeiptr size, const GLvoid* data, bool ismutable)
{
	if (buffer)
	{
		glDeleteBuffers(1, &buffer);
	}
	glGenBuffers(1, &buffer);

	if (!ismutable)
	{
		glNamedBufferStorageEXT(buffer, size, data, 0);
	}
	else
	{
		glBindBuffer(target, buffer);
		glBufferData(target, size, data, GL_STATIC_DRAW);
		glBindBuffer(target, 0);
	}

	if (bindlessVboUbo)
	{
		glGetNamedBufferParameterui64vNV(buffer, GL_BUFFER_GPU_ADDRESS_NV, &buffer64);
		glMakeNamedBufferResidentNV(buffer, GL_READ_ONLY);
	}
}

void TopazSample::initScene()
{
	// initializate skybox
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures.skybox);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		texturesAddress64.skybox = glGetTextureHandleARB(textures.skybox);
		glMakeTextureHandleResidentARB(texturesAddress64.skybox);
	}

	// init fullscreen quad
	{
	const float position[] =
	{
		-1.0f, -1.0f, 0.0,
		1.0f, -1.0f, 0.0,
		-1.0f, 1.0f, 0.0,
		1.0f, 1.0f, 0.0
	};

	initBuffer(GL_ARRAY_BUFFER, fullScreenRectangle.vboFullScreen, fullScreenRectangle.vboFullScreen64,
		sizeof(position),
		position);
}

	// initializate scene & weightBlended ubo
	{
		nv::matrix4f projection = nv::perspective(projection, 45.f * NV_PI / 180.f, m_width / float(m_height), 0.1f, 10.f);

		sceneData.projection = projection;
		sceneData.sceneDepthId64 = texturesAddress64.sceneDepth;
		sceneData.backgroundColor = sceneBackgroundColor;

		initBuffer(GL_UNIFORM_BUFFER, ubos.sceneUbo, ubos.sceneUbo64,
			sizeof(SceneData),
			&sceneData,
			true); // mutable buffer

		WeightBlendedData weightBlendedData;
		weightBlendedData.colorTex0 = oit->getAccumulationTextureId64(0);
		weightBlendedData.colorTex1 = oit->getAccumulationTextureId64(1);
		weightBlendedData.depthTexture = oit->getDepthTextureId64();

		initBuffer(GL_UNIFORM_BUFFER, ubos.weightBlendedUbo, ubos.weightBlendedUbo64,
			sizeof(WeightBlendedData),
			&weightBlendedData,
			true); // mutable buffer
	}

	brushStyle->buildBrushPattern();

	srand(1024);

	for (size_t i = 0; i < models.size(); i++)
	{
		GLfloat color = (GLfloat)(rand() & 0xff) / 0xff;

		std::unique_ptr<ObjectData> objectData(new ObjectData);
		std::unique_ptr<ObjectData> linesObjectData(new ObjectData);

		objectData->objectID = nv::vec4f((float)i);
		objectData->objectColor = nv::vec4f(color, color, color, 1.0);
		objectData->skybox = texturesAddress64.skybox;
		objectData->pattern = brushStyle->getTextureId64();

		objectData->usePattern = false;
		if (stippleMode == STIPPLE_MODE_ENABLED)
		{
			objectData->usePattern = true;
		}
		objectData->opacity = oit->getOpacity();

		models[i]->getModel()->compileModel(NvModelPrimType::TRIANGLES);

		/* ibo */
		initBuffer(GL_ELEMENT_ARRAY_BUFFER, models[i]->getBufferID("ibo"), models[i]->getBufferID64("ibo"),
			models[i]->getModel()->getCompiledIndexCount(NvModelPrimType::TRIANGLES) * sizeof(uint32_t),
			models[i]->getModel()->getCompiledIndices(NvModelPrimType::TRIANGLES));

		/* vbo */
		initBuffer(GL_ARRAY_BUFFER, models[i]->getBufferID("vbo"), models[i]->getBufferID64("vbo"),
			models[i]->getModel()->getCompiledVertexCount() * models[i]->getModel()->getCompiledVertexSize() * sizeof(float),
			models[i]->getModel()->getCompiledVertices());

		if (models[i]->cornerPointsExists())
		{
			/* ibo corner */
			initBuffer(GL_ELEMENT_ARRAY_BUFFER, models[i]->getCornerBufferID("ibo"), models[i]->getCornerBufferID64("ibo"),
				models[i]->getCornerIndices().size() * sizeof(uint32_t),
				models[i]->getCornerIndices().data());

			/* vbo corner */
			initBuffer(GL_ARRAY_BUFFER, models[i]->getCornerBufferID("vbo"), models[i]->getCornerBufferID64("vbo"),
				models[i]->getCorners().size() * sizeof(nv::vec3f),
				models[i]->getCorners().data());

			linesObjectData->objectID = nv::vec4f(1.0f);
			linesObjectData->objectColor = nv::vec4f(1.0f, 0.0f, 0.0f, 1.0);
			linesObjectData->usePattern = false; // no pattern for lines

			/* ubo corner */
			initBuffer(GL_UNIFORM_BUFFER, models[i]->getCornerBufferID("ubo"), models[i]->getCornerBufferID64("ubo"),
				sizeof(ObjectData),
				linesObjectData.get(),
				true); // mutable buffer
		}

		/* ubo */
		initBuffer(GL_UNIFORM_BUFFER, models[i]->getBufferID("ubo"), models[i]->getBufferID64("ubo"),
			sizeof(ObjectData),
			objectData.get(),
			true); // mutable buffer

		this->objectData.push_back(std::move(objectData));
		this->linesObjectData.push_back(std::move(linesObjectData));
	}
}

typedef void(*NVPproc)(void);
NVPproc sysGetProcAddress(const char* name)
{
	return (NVPproc)wglGetProcAddress(name);
}

void TopazSample::setTokenBuffers(TopazGLModel* model, std::string& stream, bool cornerPoints)
{
	GLuint vboId = (!cornerPoints) ? model->getBufferID("vbo") : model->getCornerBufferID("vbo");
	GLuint64 vboId64 = (!cornerPoints) ? model->getBufferID64("vbo") : model->getCornerBufferID64("vbo");

	GLuint iboId = (!cornerPoints) ? model->getBufferID("ibo") : model->getCornerBufferID("ibo");
	GLuint64 iboId64 = (!cornerPoints) ? model->getBufferID64("ibo") : model->getCornerBufferID64("ibo");

	GLuint uboId = (!cornerPoints) ? model->getBufferID("ubo") : model->getCornerBufferID("ubo");
	GLuint64 uboId64 = (!cornerPoints) ? model->getBufferID64("ubo") : model->getCornerBufferID64("ubo");

	NVTokenVbo vbo;
	vbo.setBinding(0);
	vbo.setBuffer(vboId, vboId64, 0);
	nvtokenEnqueue(stream, vbo);

	NVTokenIbo ibo;
	ibo.setType(GL_UNSIGNED_INT);
	ibo.setBuffer(iboId, iboId64);
	nvtokenEnqueue(stream, ibo);

	NVTokenUbo ubo;
	ubo.setBuffer(uboId, uboId64, 0, sizeof(ObjectData));
	ubo.setBinding(UBO_OBJECT, NVTOKEN_STAGE_VERTEX);
	nvtokenEnqueue(stream, ubo);
	ubo.setBinding(UBO_OBJECT, NVTOKEN_STAGE_FRAGMENT);
	nvtokenEnqueue(stream, ubo);
}

void TopazSample::pushTokenParameters(NVTokenSequence& sequence, size_t& offset, std::string& stream, GLuint fbo, GLuint state)
{
	sequence.offsets.push_back(offset);
	sequence.sizes.push_back(GLsizei(stream.size() - offset));
	sequence.fbos.push_back(fbo);
	sequence.states.push_back(state);

	offset = stream.size();
}

void TopazSample::initCommandList()
{
	if (!isTokenInternalsInited)
	{
		hwsupport = init_NV_command_list(sysGetProcAddress) ? true : false;
		nvtokenInitInternals(hwsupport, bindlessVboUbo);

		isTokenInternalsInited = true;
	}

	if (hwsupport)
	{
		for (size_t i = 0; i < STATES_COUNT; i++)
		{
			glCreateStatesNV(1, &cmdlist.stateObjects[i]);
		}

		glGenBuffers(1, &cmdlist.tokenBuffer);
		glCreateCommandListsNV(1, &cmdlist.tokenCmdList);
	}

	NVTokenSequence& seq = cmdlist.tokenSequence;
	std::string& stream = cmdlist.tokenData;
	size_t offset = 0;

	{
		NVTokenUbo  ubo;
		ubo.setBuffer(ubos.sceneUbo, ubos.sceneUbo64, 0, sizeof(SceneData));
		ubo.setBinding(UBO_SCENE, NVTOKEN_STAGE_VERTEX);
		nvtokenEnqueue(stream, ubo);
		ubo.setBinding(UBO_SCENE, NVTOKEN_STAGE_FRAGMENT);
		nvtokenEnqueue(stream, ubo);
	}

	NVTokenUbo uboWeightBlended;
	uboWeightBlended.setBuffer(ubos.weightBlendedUbo, ubos.weightBlendedUbo64, 0, sizeof(WeightBlendedData));
	uboWeightBlended.setBinding(UBO_OIT, NVTOKEN_STAGE_FRAGMENT);
	nvtokenEnqueue(stream, uboWeightBlended);

	for (auto model = models.begin(); model != models.end(); model++)
	{
		// 1. geometry pass
		{
			setTokenBuffers((*model).get(), stream);

			NVTokenDrawElems  draw;
			draw.setParams((*model)->getModel()->getCompiledIndexCount(NvModelPrimType::TRIANGLES));
			draw.setMode(GL_TRIANGLES);
			nvtokenEnqueue(stream, draw);
		}
		pushTokenParameters(seq, offset, stream, oit->getFramebufferID(), cmdlist.stateObjects[STATE_TRANSPARENT]);

		if ((*model)->cornerPointsExists())
		{
			{
				setTokenBuffers((*model).get(), stream, true);

				NVTokenDrawElems  draw;
				draw.setParams((*model)->getCornerIndices().size());
				draw.setMode(GL_LINE_STRIP);
				nvtokenEnqueue(stream, draw);
			}
			pushTokenParameters(seq, offset, stream, oit->getFramebufferID(), cmdlist.stateObjects[STATE_TRASPARENT_LINES]);
		}
	}

	// 2. composite pass
	{
		NVTokenVbo vbo;
		vbo.setBinding(0);
		vbo.setBuffer(fullScreenRectangle.vboFullScreen, fullScreenRectangle.vboFullScreen64, 0);
		nvtokenEnqueue(stream, vbo);

		NVTokenDrawArrays  draw;
		draw.setParams(4, 0);
		draw.setMode(GL_TRIANGLE_STRIP);
		nvtokenEnqueue(stream, draw);
	}
	pushTokenParameters(seq, offset, stream, fbos.scene, cmdlist.stateObjects[STATE_COMPOSITE]);

	if (hwsupport)
	{
		glNamedBufferStorageEXT(cmdlist.tokenBuffer, cmdlist.tokenData.size(), &cmdlist.tokenData[0], 0);

		cmdlist.tokenSequenceList = cmdlist.tokenSequence;
		for (size_t i = 0; i < cmdlist.tokenSequenceList.offsets.size(); i++)
		{
			cmdlist.tokenSequenceList.offsets[i] += (GLintptr)&cmdlist.tokenData[0];
		}
	}

	updateCommandListState();
}

void TopazSample::updateCommandListState()
{
	glEnableVertexAttribArray(VERTEX_POS);
	glVertexAttribFormat(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(VERTEX_POS, 0);

	size_t stride = models[0]->getModel()->getCompiledVertexSize() * sizeof(float);

	glDisable(GL_DEPTH_TEST);

	if (oit->getBlendDst() == GL_ZERO)
	{
		glEnable(GL_DEPTH_TEST);

		/* TODO: change on GL_ALWAYS, when driver issue will be fixed */
		glDepthFunc(GL_LEQUAL);
	}

	// 1. oit first step
	{
		glBindFramebuffer(GL_FRAMEBUFFER, oit->getFramebufferID());

		const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunci(0, oit->getBlendSrc(), oit->getBlendDst());
		glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

		glUseProgram(shaderPrograms["weightBlended"]->getProgram());

		glBindVertexBuffer(0, 0, 0, stride);
		glStateCaptureNV(cmdlist.stateObjects[STATE_TRANSPARENT], GL_TRIANGLES);

		glBindVertexBuffer(0, 0, 0, sizeof(nv::vec3f));
		glStateCaptureNV(cmdlist.stateObjects[STATE_TRASPARENT_LINES], GL_LINES);

		glUseProgram(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// 2. oit second step
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);

		glUseProgram(shaderPrograms["weightBlendedFinal"]->getProgram());

		glBindVertexBuffer(0, 0, 0, sizeof(nv::vec3f));
		glStateCaptureNV(cmdlist.stateObjects[STATE_COMPOSITE], GL_TRIANGLES);

		glUseProgram(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// compile command list
	NVTokenSequence& sequenceList = cmdlist.tokenSequenceList;
	glCommandListSegmentsNV(cmdlist.tokenCmdList, 1);
	glListDrawCommandsStatesClientNV(cmdlist.tokenCmdList, 0, (const void**)&sequenceList.offsets[0], &sequenceList.sizes[0], &sequenceList.states[0], &sequenceList.fbos[0], int(sequenceList.states.size()));
	glCompileCommandListNV(cmdlist.tokenCmdList);
}


void TopazSample::initFramebuffers(int32_t width, int32_t height)
{
	if (textures.sceneColor && GLEW_ARB_bindless_texture)
	{
		glMakeTextureHandleNonResidentARB(texturesAddress64.sceneColor);
		glMakeTextureHandleNonResidentARB(texturesAddress64.sceneDepth);
	}

	if (textures.sceneColor)
	{
		glDeleteTextures(1, &textures.sceneColor);
	}
	glGenTextures(1, &textures.sceneColor);

	glBindTexture(GL_TEXTURE_RECTANGLE, textures.sceneColor);
	glTexStorage2D(GL_TEXTURE_RECTANGLE, 1, GL_RGBA16F, width, height);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	if (textures.sceneDepth)
	{
		glDeleteTextures(1, &textures.sceneDepth);
	}
	glGenTextures(1, &textures.sceneDepth);

	glBindTexture(GL_TEXTURE_RECTANGLE, textures.sceneDepth);
	glTexStorage2D(GL_TEXTURE_RECTANGLE, 1, GL_DEPTH_COMPONENT32F, width, height);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	if (fbos.scene)
	{
		glDeleteFramebuffers(1, &fbos.scene);
	}
	glGenFramebuffers(1, &fbos.scene);

	glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, textures.sceneColor, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, textures.sceneDepth, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (GLEW_ARB_bindless_texture)
	{
		texturesAddress64.sceneColor = glGetTextureHandleARB(textures.sceneColor);
		texturesAddress64.sceneDepth = glGetTextureHandleARB(textures.sceneDepth);
		glMakeTextureHandleResidentARB(texturesAddress64.sceneColor);
		glMakeTextureHandleResidentARB(texturesAddress64.sceneDepth);
	}
}

void TopazSample::drawModel(GLenum mode, GLSLProgram& program, TopazGLModel& model)
{
	glVertexAttribFormat(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, model.getModel()->getCompiledPositionOffset());

	glVertexAttribBinding(VERTEX_POS, 0);
	glEnableVertexAttribArray(VERTEX_POS);

	glUseProgram(program.getProgram());

	glBindBufferRange(GL_UNIFORM_BUFFER, UBO_OBJECT, model.getBufferID("ubo"), 0, sizeof(ObjectData));

	glBindVertexBuffer(0, model.getBufferID("vbo"), 0, model.getModel()->getCompiledVertexSize() * sizeof(float));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.getBufferID("ibo"));
	glDrawElements(mode, model.getModel()->getCompiledIndexCount(NvModelPrimType::TRIANGLES), GL_UNSIGNED_INT, nullptr);

	if (model.cornerPointsExists())
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_OBJECT, model.getCornerBufferID("ubo"), 0, sizeof(ObjectData));

		glBindVertexBuffer(0, model.getCornerBufferID("vbo"), 0, sizeof(nv::vec3f));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.getCornerBufferID("ibo"));

		glDrawElements(GL_LINE_STRIP, model.getCornerIndices().size(), GL_UNSIGNED_INT, nullptr);
	}

	glUseProgram(0);

	glBindBufferRange(GL_UNIFORM_BUFFER, UBO_OBJECT, 0, 0, 0);
	glDisableVertexAttribArray(VERTEX_POS);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexBuffer(0, 0, 0, 0);
}

void TopazSample::renderStandartWeightedBlendedOIT()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_SCENE, ubos.sceneUbo);
	glBindBufferRange(GL_UNIFORM_BUFFER, UBO_OIT, ubos.weightBlendedUbo, 0, sizeof(WeightBlendedData));

	glDisable(GL_DEPTH_TEST);
	oit->setBlendDst(GL_ONE);

	if (fabs(oit->getOpacity() - 1.0) < 0.001)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		oit->setBlendDst(GL_ZERO);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, oit->getFramebufferID());

	const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	GLfloat clearColorZero[4] = { 0.0f };
	GLfloat clearColorOne[4] = { 1.0f };

	glClearBufferfv(GL_COLOR, 0, clearColorZero);
	glClearBufferfv(GL_COLOR, 1, clearColorOne);
	glClearBufferfv(GL_DEPTH, 0, clearColorOne);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunci(0, oit->getBlendSrc(), oit->getBlendDst());
	glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

	for (size_t i = 0; i < models.size() - numberOfFormulars; i++)
	{
		// geometry pass 
		{
			objectData[i]->usePattern = false;
			if (stippleMode == STIPPLE_MODE_ENABLED)
			{
				objectData[i]->usePattern = true;
			}

			objectData[i]->opacity = oit->getOpacity();
			objectData[i]->modelView = m_transformer->getModelViewMat();
			glNamedBufferSubDataEXT(models[i]->getBufferID("ubo"), 0, sizeof(ObjectData), objectData[i].get());

			if (models[i]->cornerPointsExists())
			{
				linesObjectData[i]->opacity = oit->getOpacity();
				linesObjectData[i]->modelView = m_transformer->getModelViewMat();
				glNamedBufferSubDataEXT(models[i]->getCornerBufferID("ubo"), 0, sizeof(ObjectData), linesObjectData[i].get());
			}

			drawModel(GL_TRIANGLES, *shaderPrograms["weightBlended"], *models[i]);
		}
	}

	size_t idx = models.size() - numberOfFormulars;
	for (size_t i = 1; i < models.size() - numberOfFormulars; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			if (idx == models.size())
			{
				break;
			}

			if (!models[i]->cornerPointsExists())
			{
				continue;
			}

			recalculatePositionOfFormular(i, j, idx);

			glNamedBufferSubDataEXT(models[idx]->getBufferID("ubo"), 0, sizeof(ObjectData), objectData[idx].get());
			glNamedBufferSubDataEXT(models[idx]->getCornerBufferID("ubo"), 0, sizeof(ObjectData), linesObjectData[idx].get());

			drawModel(GL_TRIANGLES, *shaderPrograms["weightBlended"], *models[idx]);

			idx++;
		}
	}

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// composition pass 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
		{
			glUseProgram(shaderPrograms["weightBlendedFinal"]->getProgram());

			glVertexAttribFormat(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, 0);

			glVertexAttribBinding(VERTEX_POS, 0);
			glEnableVertexAttribArray(VERTEX_POS);

			glBindVertexBuffer(0, fullScreenRectangle.vboFullScreen, 0, sizeof(nv::vec3f));
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glUseProgram(0);
		}
	}
}

NvAppBase* NvAppFactory(NvPlatformContext* platform)
{
	return new TopazSample(platform);
}

