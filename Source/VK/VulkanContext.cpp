#include <PCH.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/CommandPoolManager.h>
#include <VK/DescriptorManager.h>
#include <VK/FrameTracker.h>
#include <VK/LayoutCache.h>

namespace sy::vk
{
	VulkanContext::VulkanContext(const window::Window& window) :
		vulkanRHI(std::make_unique<VulkanRHI>(window)),
		frameTracker(std::make_unique<FrameTracker>(*vulkanRHI)),
		cmdPoolManager(std::make_unique<CommandPoolManager>(*vulkanRHI, *frameTracker)),
		descriptorManager(std::make_unique<DescriptorManager>(*vulkanRHI, *frameTracker)),
		pipelineLayoutCache(std::make_unique<PipelineLayoutCache>(*vulkanRHI))
	{
	}

	VulkanContext::~VulkanContext()
	{
		vulkanRHI->WaitForDeviceIdle();
	}

	CommandPoolManager& VulkanContext::GetCommandPoolManager() const
	{
		return *cmdPoolManager;
	}

	DescriptorManager& VulkanContext::GetDescriptorManager() const
	{
		return *descriptorManager;
	}

	VulkanRHI& VulkanContext::GetVulkanRHI() const
	{
		return *vulkanRHI;
	}

	FrameTracker& VulkanContext::GetFrameTracker() const
	{
		return *frameTracker;
	}

	PipelineLayoutCache& VulkanContext::GetPipelineLayoutCache() const
	{
		return *pipelineLayoutCache;
	}

	void VulkanContext::BeginFrame()
	{
		frameTracker->BeginFrame();
	}

	void VulkanContext::EndFrame()
	{
		frameTracker->EndFrame();
	}

	void VulkanContext::BeginRender()
	{
		frameTracker->WaitForInFlightRenderFence();
		cmdPoolManager->BeginFrame();
		descriptorManager->BeginFrame();
	}

	void VulkanContext::EndRender()
	{
		descriptorManager->EndFrame();
		cmdPoolManager->EndFrame();
	}
}
