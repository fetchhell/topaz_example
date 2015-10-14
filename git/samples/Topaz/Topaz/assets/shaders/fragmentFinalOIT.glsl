#version 450 core

#extension GL_ARB_shading_language_include : enable
#include "common.h"

layout(location = 0) out vec4 outColor;

void main(void)
{
    vec4 sumColor = texture(weightBlendedData.colorTex0, gl_FragCoord.xy); 
    float transmittance = texture(weightBlendedData.colorTex1, gl_FragCoord.xy).r;  
	
    vec3 averageColor = sumColor.rgb / max(sumColor.a, 0.00001);

    outColor.rgb = averageColor * (1 - transmittance) + sceneData.backgroundColor.rgb * transmittance;
}

