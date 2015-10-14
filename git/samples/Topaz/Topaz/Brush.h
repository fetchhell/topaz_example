#pragma once

#include "includeAll.h"

class BrushStyles
{
public:
	BrushStyles();

	void buildBrushPattern();

	GLuint& getTextureId()
	{
		return textureBrushStyleId;
	}

	GLuint64& getTextureId64()
	{
		return textureBrushStyleId64;
	}

private:
	GLuint textureBrushStyleId;
	GLuint64 textureBrushStyleId64;
};