#include "Application.h"

// Private

bool Application::applicationInit()
{
	return true;
}

bool Application::applicationUpdate(uint32_t frameIndex, double deltaTime, double totalTime)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Hello Imgui!");
    ImGui::Text("This is some useful text.");
	ImGui::End();

	ImGui::Render();

	//

	VkClearColorValue clearColorValue = {};
	clearColorValue.float32[0] = 0.0f;
	clearColorValue.float32[1] = 0.0f;
	clearColorValue.float32[2] = 1.0f;
	clearColorValue.float32[3] = 1.0f;

	VkClearValue clearValues[1] = {};
	clearValues[0].color = clearColorValue;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffers[frameIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = {width, height};
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(commandBuffers[frameIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Just render the imgui content.
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[frameIndex]);

	vkCmdEndRenderPass(commandBuffers[frameIndex]);

	return true;
}

void Application::applicationTerminate()
{
}

// Public

Application::Application(const char* title) :
	TinyEngine(title)
{
}

Application::~Application()
{
}
