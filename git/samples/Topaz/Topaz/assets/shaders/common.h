#define VERTEX_POS    0
#define VERTEX_NORMAL 1
#define VERTEX_UV     2

#define UBO_SCENE     0
#define UBO_OBJECT    1
#define UBO_OIT       2

#if defined(GL_core_profile) || defined(GL_compatibility_profile) || defined(GL_es_profile)

#extension GL_ARB_bindless_texture : require
#extension GL_NV_command_list 	   : require
#extension GL_NV_shadow_samplers_cube : enable

#if GL_NV_command_list
	layout(commandBindableNV) uniform;
#endif

struct SceneData
{
	mat4 projection;
	vec4 backgroundColor;
	sampler2DRect depthTexture;
	float depthScale;
};

struct WeightBlendedData
{
	sampler2DRect colorTex0;
	sampler2DRect colorTex1;
	sampler2DRect depthTexture;
};

struct ObjectData
{
	mat4 modelView;
	vec4 objectID;
	vec4 objectColor;
	samplerCube skybox;
	sampler2DRect pattern;
	int usePattern;
	float opacity;
};

layout(std140, binding = UBO_SCENE) uniform sceneBuffer
{
	SceneData  sceneData;
};

layout(std140, binding = UBO_OBJECT) uniform objectBuffer
{
	ObjectData  objectData;
};

layout(std140, binding = UBO_OIT) uniform weightBlendedBuffer
{
	WeightBlendedData weightBlendedData;
};

#endif
