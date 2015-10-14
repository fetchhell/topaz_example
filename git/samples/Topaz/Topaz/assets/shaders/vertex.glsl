#version 450 core
/**/

#extension GL_ARB_shading_language_include : enable
#include "common.h"

in layout(location = VERTEX_POS)    vec3 pos;
in layout(location = VERTEX_NORMAL) vec3 normal;
in layout(location = VERTEX_UV)     vec2 uv;

out Varyings
{
	vec3 pos;
} out_varyings;

void main()
{	
	gl_Position = sceneData.projection * objectData.modelView * vec4(pos, 1);
	out_varyings.pos = pos;
}







 