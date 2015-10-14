#include "Brush.h"

BrushStyles::BrushStyles()
{
}

void BrushStyles::buildBrushPattern()
{
	std::vector<unsigned char> pattern32(32 * 32, 0);

	for (size_t i = 0; i < 32; i+=8)
	{
		for (size_t j = 0; j < 32; j++)
		{
			pattern32[i * 32 + j] = 1;		
		}
	}

	for (size_t i = 0; i < 32; i++)
	{
		for (size_t j = 0; j < 32; j+=8)
		{
			pattern32[i * 32 + j] = 1;
		}
	}
	
	glGenTextures(1, &textureBrushStyleId);

	glBindTexture(GL_TEXTURE_RECTANGLE, textureBrushStyleId);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8, 32, 32, 0, GL_RED, GL_UNSIGNED_BYTE, pattern32.data());
	
	if (GLEW_ARB_bindless_texture)
	{
		textureBrushStyleId64 = glGetTextureHandleARB(textureBrushStyleId);
		glMakeTextureHandleResidentARB(textureBrushStyleId64);
	}
	
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
}