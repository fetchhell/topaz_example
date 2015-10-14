#version 450 core

#extension GL_ARB_shading_language_include : enable
#include "common.h"

in layout(location = VERTEX_POS)    vec3 pos;
in layout(location = VERTEX_NORMAL) vec3 normal;
in layout(location = VERTEX_UV)     vec2 uv;

void main()
{
	gl_Position = vec4(pos, 1);
}
