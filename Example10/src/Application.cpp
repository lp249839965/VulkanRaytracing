#include "Application.h"

#include "GLTF/HelperAccess.h"
#include "GLTF/HelperLoader.h"
#include "GLTF/HelperLoop.h"
#include "GLTF/HelperResource.h"

// Private

bool Application::applicationInit()
{
	HelperLoader helperLoader(width, height, physicalDevice, device, queue, commandPool, renderPass, samples);

	if(!helperLoader.open(glTF, filename, environment))
	{
		return false;
	}

	return true;
}

bool Application::applicationUpdate(uint32_t frameIndex, double deltaTime, double totalTime)
{
	VkClearColorValue resolveClearColorValue = {};
	resolveClearColorValue.float32[0] = 0.0f;
	resolveClearColorValue.float32[1] = 0.0f;
	resolveClearColorValue.float32[2] = 0.0f;
	resolveClearColorValue.float32[3] = 0.0f;

	VkClearColorValue clearColorValue = {};
	clearColorValue.float32[0] = 0.0f;
	clearColorValue.float32[1] = 0.0f;
	clearColorValue.float32[2] = 0.0f;
	clearColorValue.float32[3] = 1.0f;

	VkClearDepthStencilValue clearDepthStencilValue = {};
	clearDepthStencilValue.depth = 1.0f;

	std::vector<VkClearValue> clearValues;
	if (samples == VK_SAMPLE_COUNT_1_BIT)
	{
		clearValues.resize(2);
		clearValues[0].color = clearColorValue;
		clearValues[1].depthStencil = clearDepthStencilValue;
	}
	else
	{
		clearValues.resize(3);
		clearValues[0].color = resolveClearColorValue;
		clearValues[1].color = clearColorValue;
		clearValues[2].depthStencil = clearDepthStencilValue;
	}

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffers[frameIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = {width, height};
	renderPassBeginInfo.clearValueCount = clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffers[frameIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	glTF.viewProjection.projection = Math::perspective(45.0f, (float)width/(float)height, 0.1f, 100.0f);

	glm::mat3 orbitMatrix = glm::rotate(rotY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(rotX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 orbitEye = orbitMatrix * glm::vec3(0.0f, 0.0f, eyeObjectDistance);

	glTF.viewProjection.view = glm::lookAt(orbitEye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	HelperLoop::draw(glTF, commandBuffers[frameIndex], frameIndex);

	vkCmdEndRenderPass(commandBuffers[frameIndex]);

	return true;
}

void Application::applicationTerminate()
{
	HelperResource::terminate(glTF, device);
}

// Public

Application::Application(const char* title, const std::string& filename, const std::string& environment) :
	TinyEngine(title), filename(filename), environment(environment)
{
}

Application::~Application()
{
}

void Application::orbitY(float orbit)
{
	rotY -= orbit * 0.01f;
}

void Application::orbitX(float orbit)
{
	rotX -= orbit * 0.01f;

	// Not more than 90 degrees
	rotX = glm::clamp(rotX, -glm::pi<float>() * 0.49f, glm::pi<float>() * 0.49f);
}

void Application::zoom(float zoom)
{
	eyeObjectDistance += zoom * 0.05;

	eyeObjectDistance = glm::clamp(eyeObjectDistance, 0.001f, 1000.0f);
}
