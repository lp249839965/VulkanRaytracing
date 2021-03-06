#version 460 core
#extension GL_EXT_ray_tracing : enable

layout (binding = SPECULAR_BINDING) uniform samplerCube u_specularTexture;

struct Payload
{
  vec3 color;
  uint depth;
  uint maxDepth;
  bool primitive;
  vec3 ray;
};

layout(location = 0) rayPayloadInEXT Payload out_hitValue;

void main()
{
	if (!out_hitValue.primitive)
	{
	    out_hitValue.color = vec3(0.0, 0.0, 0.0);
	}
	else
	{
		out_hitValue.color = textureLod(u_specularTexture, out_hitValue.ray, 0.0).rgb;
	}
}