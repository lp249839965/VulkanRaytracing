#ifndef HELPER_H_
#define HELPER_H_

#include <vector>

#define VK_ENABLE_BETA_EXTENSIONS
#include "volk.h"

struct BufferResourceCreateInfo {
    VkDeviceSize           size = 0;
    VkBufferUsageFlags     usage = 0;

	VkMemoryPropertyFlags  memoryProperty = 0;
	const void* pNext = nullptr;
};

struct BufferResource {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
};

struct ImageDataResource {
	std::vector<uint8_t> pixels;
	uint32_t width = 0;
	uint32_t height = 0;
	VkFormat format = VK_FORMAT_UNDEFINED;

	uint32_t mipLevel = 0;
	uint32_t face = 0;
};

struct ImageDataResources {
	std::vector<ImageDataResource> images = std::vector<ImageDataResource>(1);
	uint32_t mipLevels = 1;
	uint32_t faceCount = 1;
};

struct ImageViewResourceCreateInfo {
	VkImageCreateFlags flags = 0;
	VkImageType imageType = VK_IMAGE_TYPE_2D;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkExtent3D extent = {0, 0, 0};
	uint32_t mipLevels = 1;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageUsageFlags usage = 0;

	VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	VkImageAspectFlags aspectMask = 0;
};

struct ImageViewResource {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
};

//

struct DeviceBufferResourceCreateInfo {
	BufferResourceCreateInfo bufferResourceCreateInfo = {};

    const void* data = nullptr;
};

struct DeviceBufferResource {
	BufferResource bufferResource;
};

struct VertexBufferResourceCreateInfo : DeviceBufferResourceCreateInfo {
};

struct VertexBufferResource : DeviceBufferResource {
};

struct StorageBufferResourceCreateInfo : DeviceBufferResourceCreateInfo {
};

struct StorageBufferResource : DeviceBufferResource {
};

//

struct HostBufferResourceCreateInfo {
	BufferResourceCreateInfo bufferResourceCreateInfo = {0, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    const void* data = nullptr;
};

struct HostBufferResource {
	BufferResource bufferResource;
};

struct UniformBufferResourceCreateInfo : HostBufferResourceCreateInfo {
};

struct UniformBufferResource : HostBufferResource {
};

//

struct SamplerResourceCreateInfo {
    VkFilter                magFilter = VK_FILTER_LINEAR;
    VkFilter                minFilter = VK_FILTER_LINEAR;
    VkSamplerMipmapMode     mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    VkSamplerAddressMode    addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode    addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    float                   minLod = 0.0f;
    float                   maxLod = 1.0f;
};

struct SamplerResource {
	VkSampler sampler = VK_NULL_HANDLE;
};

struct TextureResourceCreateInfo {
	ImageDataResources imageDataResources = {};
	SamplerResourceCreateInfo samplerResourceCreateInfo = {};
	bool mipMap = false;
};

struct TextureResource {
	ImageViewResource imageViewResource = {};
	SamplerResource samplerResource = {};
};

class Helper
{
public:

	static bool isDepthStencilFormat(VkFormat format);

	static bool getFormat(VkFormat& format, uint32_t componentTypeSize, bool componentTypeSigned, bool componentTypeInteger, uint32_t typeCount);

	static bool findMemoryTypeIndex(uint32_t& memoryTypeIndex, VkPhysicalDevice physicalDevice, uint32_t memoryType, VkMemoryPropertyFlags memoryProperty);

	static bool beginOneTimeSubmitCommand(VkDevice device, VkCommandPool commandPool, VkCommandBuffer& commandBuffer);

	static bool endOneTimeSubmitCommand(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

	static bool transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel, uint32_t levelCount, uint32_t layerCount);

	static bool copyBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	static bool copyBufferToImage(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t baseArrayLayer);

	static bool generateMipMap(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, uint32_t width, uint32_t height, uint32_t levelCount, uint32_t layerCount);

	static bool copyHostToDevice(VkDevice device, BufferResource& bufferResource, const void* data, size_t size);

	static bool createBufferResource(VkPhysicalDevice physicalDevice, VkDevice device, BufferResource& bufferResource, const BufferResourceCreateInfo& bufferResourceCreateInfo);

	static void destroyBufferResource(VkDevice device, BufferResource& bufferResource);

	static bool createShaderModule(VkShaderModule& shaderModule, VkDevice device, const std::vector<uint32_t>& code);

	static bool createDeviceBufferResource(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool commandPool, DeviceBufferResource& deviceBufferResource, const DeviceBufferResourceCreateInfo& deviceBufferResourceCreateInfo);

	static void destroyDeviceBufferResource(VkDevice device, DeviceBufferResource& deviceBufferResource);

	static bool createVertexBufferResource(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool commandPool, VertexBufferResource& vertexBufferResource, const VertexBufferResourceCreateInfo& vertexBufferResourceCreateInfo);

	static void destroyVertexBufferResource(VkDevice device, VertexBufferResource& vertexBufferResource);

	static bool createStorageBufferResource(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool commandPool, StorageBufferResource& storageBufferResource, const StorageBufferResourceCreateInfo& storageBufferResourceCreateInfo);

	static void destroyStorageBufferResource(VkDevice device, StorageBufferResource& storageBufferResource);

	static bool createUniformBufferResource(VkPhysicalDevice physicalDevice, VkDevice device, UniformBufferResource& uniformBufferResource, const UniformBufferResourceCreateInfo& uniformBufferResourceCreateInfo);

	static void destroyUniformBufferResource(VkDevice device, UniformBufferResource& uniformBufferResource);

	static bool createImageViewResource(VkPhysicalDevice physicalDevice, VkDevice device, ImageViewResource& imageViewResource, const ImageViewResourceCreateInfo& imageViewResourceCreateInfo);

	static void destroyImageViewResource(VkDevice device, ImageViewResource& imageViewResource);

	static bool createSamplerResource(VkDevice device, SamplerResource& samplerResource, const SamplerResourceCreateInfo& samplerResourceCreateInfo);

	static void destroySamplerResource(VkDevice device, SamplerResource& samplerResource);

	static bool createTextureResource(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool commandPool, TextureResource& textureResource, const TextureResourceCreateInfo& textureResourceCreateInfo);

	static void destroyTextureResource(VkDevice device, TextureResource& textureResource);

};

#endif /* HELPER_H_ */
