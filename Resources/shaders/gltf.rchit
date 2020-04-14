#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform UniformPushConstant {
    mat4 inverseProjection;
    mat4 inverseView;
} in_upc;

struct UniformBuffer {
	vec4 baseColorFactor;

	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	vec3 emissiveFactor;
	uint alphaMode;

	float alphaCutoff;
	bool doubleSided;
};

struct UniformBufferRaytrace {
	UniformBuffer uniformBuffer;

	int baseColorTexture;
	int metallicRoughnessTexture;

	int emissiveTexture;

	int occlusionTexture;

	int normalTexture;
	
	//
	
	int padding;
};

struct InstanceResource {
	mat4 worldMatrix;
	int materialIndex;
	int componentTypeSize;
	
	int normalInstanceID;
	int tangentInstanceID;
	int texCoord0InstanceID;
	
	//
	
	int padding[3];
};


layout (binding = DIFFUSE_BINDING) uniform samplerCube u_diffuseTexture;
layout (binding = SPECULAR_BINDING) uniform samplerCube u_specularTexture;
layout (binding = LUT_BINDING) uniform sampler2D u_lutTexture;

layout (binding = TEXTURES_BINDING) uniform sampler2D u_textures[];
layout (binding = MATERIALS_BINDING, set = 0, scalar) buffer Materials { UniformBufferRaytrace ubf[]; } u_materials;
layout (binding = PRIMITIVES_BINDING, set = 0, scalar) buffer InstanceResources { InstanceResource i[]; } u_instanceResources;

layout (binding = INDICES_BINDING, set = 0, scalar) buffer Indices { uint i[]; } u_indices[];
layout (binding = POSITION_BINDING, set = 0, scalar) buffer Position { float i[]; } u_position[];
#ifdef NORMAL_VEC3
layout (binding = NORMAL_BINDING, set = 0, scalar) buffer Normal { float i[]; } u_normal[];
#endif
#ifdef TANGENT_VEC4
layout (binding = TANGENT_BINDING, set = 0, scalar) buffer Tangent { vec4 i[]; } u_tangent[];
#endif
#ifdef TEXCOORD0_VEC2
layout (binding = TEXCOORD0_BINDING, set = 0, scalar) buffer TexCoord0 { vec2 i[]; } u_texCoord0[];
#endif

hitAttributeEXT vec3 in_hitAttribute;

layout(location = 0) rayPayloadInEXT vec3 out_hitValue;

//

void computeTangent(in vec3 pos0, in vec3 pos1, in vec3 pos2, in vec2 uv0, in vec2 uv1, in vec2 uv2, in vec3 geo_normal, out vec3 tangent, out vec3 binormal)
{
	// Tangent and Binormal
	// http://www.terathon.com/code/tangent.html
	// https://github.com/nvpro-samples/vk_raytrace/blob/master/shaders/raytrace.rchit
	float x1 = pos1.x - pos0.x;
	float x2 = pos2.x - pos0.x;
	float y1 = pos1.y - pos0.y;
	float y2 = pos2.y - pos0.y;
	float z1 = pos1.z - pos0.z;
	float z2 = pos2.z - pos0.z;

	float s1 = uv1.x - uv0.x;
	float s2 = uv2.x - uv0.x;
	float t1 = uv1.y - uv0.y;
	float t2 = uv2.y - uv0.y;

	float r    = 1.0 / (s1 * t2 - s2 * t1);
	vec3  sdir = vec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
	vec3  tdir = vec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

	vec3  N          = normalize(geo_normal);
	vec3  T          = normalize(sdir - N * dot(N, sdir));
	vec3  B          = normalize(cross(N, T));

	tangent  = T;
	binormal = B;
}

//

uint getIndex(int i, int componentTypeSize)
{
	if (componentTypeSize == 2)
	{
		uint shift = 0;
	
		if ((gl_PrimitiveID % 2 == 0 && i == 1) || (gl_PrimitiveID % 2 == 1 && i != 1))
		{
			shift = 16;
		}
	
		return (u_indices[gl_InstanceID].i[(gl_PrimitiveID * 3 + i) / 2] >> shift) & 0xFFFF;
	}
	
	return u_indices[gl_InstanceID].i[gl_PrimitiveID * 3 + i];
}

vec4 getPosition(uvec3 indices, vec3 barycentrics)
{
	vec3 position_x = vec3(u_position[gl_InstanceID].i[3 * indices.x + 0], u_position[gl_InstanceID].i[3 * indices.x + 1], u_position[gl_InstanceID].i[3 * indices.x + 2]);
	vec3 position_y = vec3(u_position[gl_InstanceID].i[3 * indices.y + 0], u_position[gl_InstanceID].i[3 * indices.y + 1], u_position[gl_InstanceID].i[3 * indices.y + 2]);
	vec3 position_z = vec3(u_position[gl_InstanceID].i[3 * indices.z + 0], u_position[gl_InstanceID].i[3 * indices.z + 1], u_position[gl_InstanceID].i[3 * indices.z + 2]);

	return vec4(position_x * barycentrics.x + position_y * barycentrics.y + position_z * barycentrics.z, 1.0);
}

vec3 getNormal(uvec3 indices, vec3 barycentrics, int normalInstanceID, int tangentInstanceID, int texCoord0InstanceID, int materialIndex, vec2 st)
{
	vec3 normal = vec3(0.0, 1.0, 0.0); 

	if (normalInstanceID < 0)
	{
		return normal;
	}

#ifdef NORMAL_VEC3
	vec3 normal_x = vec3(u_normal[normalInstanceID].i[3 * indices.x + 0], u_normal[normalInstanceID].i[3 * indices.x + 1], u_normal[normalInstanceID].i[3 * indices.x + 2]);
	vec3 normal_y = vec3(u_normal[normalInstanceID].i[3 * indices.y + 0], u_normal[normalInstanceID].i[3 * indices.y + 1], u_normal[normalInstanceID].i[3 * indices.y + 2]);
	vec3 normal_z = vec3(u_normal[normalInstanceID].i[3 * indices.z + 0], u_normal[normalInstanceID].i[3 * indices.z + 1], u_normal[normalInstanceID].i[3 * indices.z + 2]);

	normal = normalize(normal_x * barycentrics.x + normal_y * barycentrics.y + normal_z * barycentrics.z);
#endif

	if (u_materials.ubf[materialIndex].normalTexture >= 0)
	{
	    vec3 n = texture(u_textures[u_materials.ubf[materialIndex].normalTexture], st).rgb;
	    n = normalize((2.0 * n - 1.0) * vec3(u_materials.ubf[materialIndex].uniformBuffer.normalScale, u_materials.ubf[materialIndex].uniformBuffer.normalScale, 1.0));
	    
	    vec3 tangent = vec3(1.0, 0.0, 0.0);
	    vec3 bitangent = vec3(0.0, 0.0, 1.0);
	    
	    if (tangentInstanceID >= 0)
	   	{
#ifdef TANGENT_VEC4
	    	vec4 tangent_x = u_tangent[tangentInstanceID].i[indices.x];
	    	vec4 tangent_y = u_tangent[tangentInstanceID].i[indices.y];
	    	vec4 tangent_z = u_tangent[tangentInstanceID].i[indices.z];
	
			vec4 t = tangent_x * barycentrics.x + tangent_y * barycentrics.y + tangent_z * barycentrics.z;
			
			tangent = normalize(t.xyz);
			bitangent = cross(normal, tangent) * t.w;
#endif
	   	}
	    else
	    {
#ifdef TEXCOORD0_VEC2
			vec3 pos0 = vec3(u_position[gl_InstanceID].i[3 * indices.x + 0], u_position[gl_InstanceID].i[3 * indices.x + 1], u_position[gl_InstanceID].i[3 * indices.x + 2]);
			vec3 pos1 = vec3(u_position[gl_InstanceID].i[3 * indices.y + 0], u_position[gl_InstanceID].i[3 * indices.y + 1], u_position[gl_InstanceID].i[3 * indices.y + 2]);
			vec3 pos2 = vec3(u_position[gl_InstanceID].i[3 * indices.z + 0], u_position[gl_InstanceID].i[3 * indices.z + 1], u_position[gl_InstanceID].i[3 * indices.z + 2]);
	    	vec2 uv0 = u_texCoord0[texCoord0InstanceID].i[indices.x];
	    	vec2 uv1 = u_texCoord0[texCoord0InstanceID].i[indices.y];
	    	vec2 uv2 = u_texCoord0[texCoord0InstanceID].i[indices.z];
	    
	    	computeTangent(pos0, pos1, pos2, uv0, uv1, uv2, normal, tangent, bitangent);
#endif
	    }
	    
	    mat3 tbn = mat3(tangent, bitangent, normal);
	    
	    normal = tbn * n;
	}

	// TODO: Double sided.
	
	return normal;
}

vec2 getTexCoord0(uvec3 indices, vec3 barycentrics, int texCoord0InstanceID)
{
	vec2 texCoord0 = vec2(0.0, 0.0);

	if (texCoord0InstanceID < 0)
	{
		return texCoord0;
	}
#ifdef TEXCOORD0_VEC2	
	vec2 texCoord0_x = u_texCoord0[texCoord0InstanceID].i[indices.x];
	vec2 texCoord0_y = u_texCoord0[texCoord0InstanceID].i[indices.y];
	vec2 texCoord0_z = u_texCoord0[texCoord0InstanceID].i[indices.z];
	
	texCoord0 = texCoord0_x * barycentrics.x + texCoord0_y * barycentrics.y + texCoord0_z * barycentrics.z; 
#endif
	return texCoord0;
}

//

vec3 toLinear(vec3 color)
{
    return pow(color, vec3(2.2));
}

vec3 toNonLinear(vec3 color)
{
    return pow(color, vec3(1.0/2.2));
}

//

vec3 getLambertian(vec3 normal, vec3 diffuseColor)
{
    return diffuseColor * texture(u_diffuseTexture, normal).rgb;
}

vec3 getSpecular(vec3 normal, vec3 view, float alpha, vec3 f0)
{
    float NdotV = dot(normal, view);
    
    float mipLevels = float(textureQueryLevels(u_specularTexture) - 1);
    vec3 reflection = normalize(reflect(-view, normal));
    float lod = clamp(alpha * mipLevels, 0.0, mipLevels);
    vec3 specularSample = textureLod(u_specularTexture, reflection, lod).rgb;

    vec2 brdfSamplePoint = clamp(vec2(NdotV, alpha), vec2(0.0, 0.0), vec2(1.0, 1.0));
    vec2 brdf = texture(u_lutTexture, brdfSamplePoint).rg;

    return specularSample * (f0 * brdf.x + brdf.y);
}

vec4 getBaseColor(int materialIndex, vec2 st)
{
	vec4 baseColor = u_materials.ubf[materialIndex].uniformBuffer.baseColorFactor;
	if (u_materials.ubf[materialIndex].baseColorTexture >= 0)
	{
		vec4 baseColorTexture = texture(u_textures[u_materials.ubf[materialIndex].baseColorTexture], st).rgba;

	    baseColor.rgb *= toLinear(baseColorTexture.rgb);
	    baseColor.a *= baseColorTexture.a;
	}

    return baseColor;
}

float getMetallic(int materialIndex, vec2 st)
{
	float metallic = u_materials.ubf[materialIndex].uniformBuffer.metallicFactor;

	if (u_materials.ubf[materialIndex].metallicRoughnessTexture >= 0)
	{
		metallic *= texture(u_textures[u_materials.ubf[materialIndex].metallicRoughnessTexture], st).b;
	}
	
	return metallic;
}

float getRoughness(int materialIndex, vec2 st)
{
	float roughness = u_materials.ubf[materialIndex].uniformBuffer.roughnessFactor;

	if (u_materials.ubf[materialIndex].metallicRoughnessTexture >= 0)
	{
		roughness *= texture(u_textures[u_materials.ubf[materialIndex].metallicRoughnessTexture], st).g;
	}
	
	return roughness;
}

vec3 getEmissive(int materialIndex, vec2 st)
{
    vec3 emissive = u_materials.ubf[materialIndex].uniformBuffer.emissiveFactor;
	
	if (u_materials.ubf[materialIndex].emissiveTexture >= 0)
	{
		emissive *= toLinear(texture(u_textures[u_materials.ubf[materialIndex].emissiveTexture], st).rgb);
	}

    return emissive;
}

float getOcclusion(int materialIndex, vec2 st)
{
    float occlusion = 1.0;

	if (u_materials.ubf[materialIndex].occlusionTexture >= 0)
	{
    	occlusion = texture(u_textures[u_materials.ubf[materialIndex].occlusionTexture], st).r;
	}
	
    return occlusion;
}

void main()
{
	vec3 barycentrics = vec3(1.0 - in_hitAttribute.x - in_hitAttribute.y, in_hitAttribute.x, in_hitAttribute.y);
	
	//

	mat4 worldMatrix = u_instanceResources.i[gl_InstanceID].worldMatrix;
	mat3 normalWorldMatrix = transpose(inverse(mat3(worldMatrix)));

	int materialIndex = u_instanceResources.i[gl_InstanceID].materialIndex;
	
	int componentTypeSize = u_instanceResources.i[gl_InstanceID].componentTypeSize;
	
	int normalInstanceID = u_instanceResources.i[gl_InstanceID].normalInstanceID;
	
	int tangentInstanceID = u_instanceResources.i[gl_InstanceID].tangentInstanceID;
	
	int texCoord0InstanceID = u_instanceResources.i[gl_InstanceID].texCoord0InstanceID;
		
	//
	
	uvec3 indices = uvec3(getIndex(0, componentTypeSize), getIndex(1, componentTypeSize), getIndex(2, componentTypeSize));

	vec4 position = worldMatrix * getPosition(indices, barycentrics);

	vec2 texCoord0 = getTexCoord0(indices, barycentrics, texCoord0InstanceID);
	
	vec3 normal = normalWorldMatrix * getNormal(indices, barycentrics, normalInstanceID, tangentInstanceID, texCoord0InstanceID, materialIndex, texCoord0);
		
	//
	
	vec3 view = normalize(mat3(in_upc.inverseView) * vec3(0.0, 0.0, 1.0));
	
	//
	
	vec4 baseColor = getBaseColor(materialIndex, texCoord0);
	
    float alphaChannel = baseColor.a;
    if (u_materials.ubf[materialIndex].uniformBuffer.alphaMode == 0)
    {
        alphaChannel = 1.0;
    }
    else if (u_materials.ubf[materialIndex].uniformBuffer.alphaMode == 1)
    {
        if(alphaChannel < u_materials.ubf[materialIndex].uniformBuffer.alphaCutoff)
        {
            out_hitValue = vec3(0.0, 0.0, 0.0);
            return;
        }
        alphaChannel = 1.0;
    }
	
	vec3 f0 = vec3(0.04);
	
    float metallic = getMetallic(materialIndex, texCoord0);
    vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0) * (1.0 - metallic);
    f0 = mix(f0, baseColor.rgb, metallic);

    float roughness = getRoughness(materialIndex, texCoord0);
	
	// BRDF
    vec3 color = getEmissive(materialIndex, texCoord0) + getLambertian(normal, diffuseColor) + getSpecular(normal, view, roughness, f0);
	
    // Ambient occlusion
    color = mix(color, color * getOcclusion(materialIndex, texCoord0), u_materials.ubf[materialIndex].uniformBuffer.occlusionStrength);
	
	// Gamma correction, as raytracing into non-SRGB framebuffer.
    out_hitValue = toNonLinear(color);
}