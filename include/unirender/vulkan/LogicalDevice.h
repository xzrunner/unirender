#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <cstdint>


namespace ur
{
namespace vulkan
{

class PhysicalDevice;
class Surface;

class LogicalDevice : noncopyable
{
public:
	LogicalDevice(bool enable_validation_layers,
		const PhysicalDevice& phy_dev, const Surface* surface = nullptr);
	~LogicalDevice();

	auto GetHandler() const { return m_handle; }

	auto GetGraphicsQueue() const { return m_graphics_queue; }
	auto GetPresentQueue() const { return m_present_queue; }

	// ---- Deferred resource destruction --------------------------------------
	// A dynamic vertex/index buffer re-uploaded mid-frame (Buffer::Create -> Clear)
	// must NOT free the old VkBuffer immediately: an earlier draw already recorded
	// into the frame's command buffer may still reference it once the GPU runs.
	// Instead the old handles are RETIRED, tagged with the CPU frame index in which
	// they were retired, and freed only once vulkan::Context confirms -- via that
	// frame's fence -- that every frame which could reference them has finished.
	void SetCurrentFrame(uint64_t frame) { m_cur_frame = frame; }
	void RetireBuffer(VkBuffer buffer, VkDeviceMemory memory);
	// Free everything retired in a frame <= completed_frame (its fence has signalled).
	void CollectRetired(uint64_t completed_frame);
	// Free everything regardless of frame. Caller MUST ensure the GPU is idle.
	void CollectAllRetired();

private:
	VkDevice m_handle = VK_NULL_HANDLE;

	VkQueue m_graphics_queue = VK_NULL_HANDLE;
	VkQueue m_present_queue  = VK_NULL_HANDLE;

	struct RetiredResource {
		VkBuffer       buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		uint64_t       frame  = 0;
	};
	std::vector<RetiredResource> m_retired;
	uint64_t m_cur_frame = 0;

}; // LogicalDevice

}
}