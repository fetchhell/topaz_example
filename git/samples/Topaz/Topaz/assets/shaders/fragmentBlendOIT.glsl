#version 450 core

#extension GL_ARB_shading_language_include : enable
#include "common.h"

in Varyings
{
	vec3 pos;
} in_varyings;

layout(location = 0) out vec4 oSumColor;
layout(location = 1) out vec4 oSumWeight;

void main(void)
{
	float eps = 0.0001;
 	
	// TODO: fix in driver
	if (abs(objectData.opacity - 1.0) < eps)
	{	
		float deltaDepth = texture(weightBlendedData.depthTexture, gl_FragCoord.xy).r - gl_FragCoord.z;	
		if(deltaDepth < -1e-6)
		{
			discard;
		} 		            
	}	
	
	if (objectData.usePattern == 1)
	{
		float alpha = texture(objectData.pattern, ivec2(gl_FragCoord.xy) % 32).r;
		if (alpha < eps)
		{
			discard;
		}
	}

	vec4 color = objectData.objectColor;

	if(objectData.objectID.r == 0)
	{
		color = textureCube(objectData.skybox, in_varyings.pos);
	}

	float viewDepth = abs(1.0 / gl_FragCoord.w);

	float linearDepth = viewDepth * sceneData.depthScale;
	float weight = clamp(0.03 / (1e-5 + pow(linearDepth, 4.0)), 1e-2, 3e3);

	oSumColor = vec4(color.rgb * objectData.opacity, objectData.opacity) * weight;
	oSumWeight = vec4(objectData.opacity);
}
