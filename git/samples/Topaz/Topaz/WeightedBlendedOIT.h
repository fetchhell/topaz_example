#pragma once

#include "includeAll.h"

class WeightedBlendedOIT
{
public:
	WeightedBlendedOIT();

	void InitAccumulationRenderTargets(int32_t width, int32_t height);
	void DeleteAccumulationRenderTargets();

	GLuint getFramebufferID()
	{
		return accumulationFramebufferId;
	}

	GLfloat & getOpacity()
	{
		return opacity;
	}

	GLfloat & getWeightParameter()
	{
		return weightParameter;
	}

	GLuint getAccumulationTextureId(size_t id)
	{
		return accumulationTextureId[id];
	}

	GLuint64 getAccumulationTextureId64(size_t id)
	{
		return accumulationTextureId64[id];
	}

	GLuint getDepthTextureId()
	{
		return depthTextureId;
	}

	GLuint64 getDepthTextureId64()
	{
		return depthTextureId64;
	}

	GLenum getBlendDst()
	{
		return blendDst;
	}

	GLenum getBlendSrc()
	{
		return blendSrc;
	}

	void setBlendDst(GLenum dst)
	{
		blendDst = dst;
	}

	void setBlendSrc(GLenum src)
	{
		blendSrc = src;
	}

private:

	GLuint accumulationTextureId[2];
	GLuint64 accumulationTextureId64[2];

	GLuint depthTextureId;
	GLuint64 depthTextureId64;

	GLuint accumulationFramebufferId;

	GLfloat opacity, weightParameter;

	GLenum blendDst, blendSrc;

	int imageWidth, imageHeight;
};