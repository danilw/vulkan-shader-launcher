/*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


//CODE BASED ON BASIC TUTORIAL


#define VK_USE_PLATFORM_XCB_KHR

//#define VK_USE_PLATFORM_WIN32_KHR

// I keep WAYLAND_KHR and did not test it
//#define VK_USE_PLATFORM_WAYLAND_KHR

//use pthread for keyboard/mouse/events on linux
//#define USE_PTHREAD

//OpenAL for sound
//#define USE_OPENAL





#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include <math.h>
#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <X11/Xutil.h>
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <linux/input.h>
#endif


#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#define APP_NAME_STR_LEN 80
#endif  // _WIN32

#include <vulkan/vulkan.h>

#define MILLION 1000000L
#define BILLION 1000000000L

#define APP_SHORT_NAME "VKme"
#define APP_LONG_NAME "Vulkan Demo"

// Allow a maximum of two outstanding presentation operations.
#define FRAME_LAG 2

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#if defined(NDEBUG) && defined(__GNUC__)
#define U_ASSERT_ONLY __attribute__((unused))
#else
#define U_ASSERT_ONLY
#endif

#if defined(__GNUC__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif


#ifdef _WIN32

bool in_callback = false;
#define ERR_EXIT(err_msg, err_class)                                             \
    clean_on_exit();                                                             \
    do {                                                                         \
        if (!demo->suppress_popups) MessageBox(NULL, err_msg, err_class, MB_OK); \
        exit(1);                                                                 \
    } while (0)

#else
#define ERR_EXIT(err_msg, err_class) \
    clean_on_exit();                 \
    do {                             \
        printf("%s\n", err_msg);     \
        fflush(stdout);              \
        exit(1);                     \
    } while (0)
#endif

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                              \
    {                                                                                                         \
        demo->fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint);             \
        if (demo->fp##entrypoint == NULL) {                                                                   \
            ERR_EXIT("vkGetInstanceProcAddr failed to find vk" #entrypoint, "vkGetInstanceProcAddr Failure"); \
        }                                                                                                     \
    }

static PFN_vkGetDeviceProcAddr g_gdpa = NULL;

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                                                    \
    {                                                                                                            \
        if (!g_gdpa) g_gdpa = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(demo->inst, "vkGetDeviceProcAddr"); \
        demo->fp##entrypoint = (PFN_vk##entrypoint)g_gdpa(dev, "vk" #entrypoint);                                \
        if (demo->fp##entrypoint == NULL) {                                                                      \
            ERR_EXIT("vkGetDeviceProcAddr failed to find vk" #entrypoint, "vkGetDeviceProcAddr Failure");        \
        }                                                                                                        \
    }

static int validation_error = 0;

struct vktex_all_vs_uniform {
	float position[2 * 3][4];
	float u_time[2];
	float u_resolution[2];
	float draw_id[2];
	float draw_pos[2];
	float u_mouse[2];
	float u_valt[2];
	float u_hpdata[4];
	float u_dataxx[4];
};

static const float g_uv_buffer_data[] = {

	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, -1.0f,
};


void dumpVec4(const char *note, float *vector) {
	printf("%s: \n", note);
	printf("%f, %f, %f, %f\n", vector[0], vector[1], vector[2], vector[3]);
	printf("\n");
	fflush(stdout);
}

typedef struct {
	VkImage image;
	VkCommandBuffer cmd;
	VkCommandBuffer graphics_to_present_cmd;
	VkImageView view;
	VkBuffer uniform_buffer;
	VkDeviceMemory uniform_memory;
	VkFramebuffer framebuffer;
	VkDescriptorSet descriptor_set;
} SwapchainImageResources;

struct demo {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define APP_NAME_STR_LEN 80
	HINSTANCE connection;         // hInstance - Windows Instance
	char name[APP_NAME_STR_LEN];  // Name to put on the window/icon
	HWND window;                  // hWnd - window handle
	POINT minsize;                // minimum window size
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	Display *display;
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t xcb_window;
	xcb_intern_atom_reply_t *atom_wm_delete_window;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_surface *window;
	struct wl_shell *shell;
	struct wl_shell_surface *shell_surface;
	struct wl_seat *seat;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;
#endif
	VkSurfaceKHR surface;
	bool prepared;
	bool separate_present_queue;
	bool is_minimized;


	bool syncd_with_actual_presents;
	uint64_t refresh_duration;
	uint64_t refresh_duration_multiplier;
	uint64_t target_IPD;  // image present duration (inverse of frame rate)
	uint64_t prev_desired_present_time;
	uint32_t next_present_id;
	uint32_t last_early_id;  // 0 if no early images
	uint32_t last_late_id;   // 0 if no late images

	VkInstance inst;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	uint32_t graphics_queue_family_index;
	uint32_t present_queue_family_index;
	VkSemaphore image_acquired_semaphores[FRAME_LAG];
	VkSemaphore draw_complete_semaphores[FRAME_LAG];
	VkSemaphore image_ownership_semaphores[FRAME_LAG];
	VkPhysicalDeviceProperties gpu_props;
	VkQueueFamilyProperties *queue_props;
	VkPhysicalDeviceMemoryProperties memory_properties;

	uint32_t enabled_extension_count;
	uint32_t enabled_layer_count;
	char *extension_names[64];
	char *enabled_layers[64];

	int width, height;
	VkFormat format;
	VkColorSpaceKHR color_space;

	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
	PFN_vkGetRefreshCycleDurationGOOGLE fpGetRefreshCycleDurationGOOGLE;
	PFN_vkGetPastPresentationTimingGOOGLE fpGetPastPresentationTimingGOOGLE;
	uint32_t swapchainImageCount;
	VkSwapchainKHR swapchain;
	SwapchainImageResources *swapchain_image_resources;
	VkPresentModeKHR presentMode;
	VkFence fences[FRAME_LAG];
	int frame_index;

	VkCommandPool cmd_pool;
	VkCommandPool present_cmd_pool;

	struct {
		VkFormat format;

		VkImage image;
		VkMemoryAllocateInfo mem_alloc;
		VkDeviceMemory mem;
		VkImageView view;
	} depth;

	VkCommandBuffer cmd;  // Buffer for initialization commands
	VkPipelineLayout pipeline_layout;
	VkDescriptorSetLayout desc_layout;
	VkPipelineCache pipelineCache;
	VkRenderPass render_pass;
	VkPipeline pipeline;


	float spin_angle;
	float spin_angle_val;
	float spin_increment;
	bool pause;

	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	VkDescriptorPool desc_pool;

	bool quit;
	int32_t curFrame;
	int32_t frameCount;
	bool suppress_popups;

	PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
	PFN_vkSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT;
	PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
	PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;
	PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
	VkDebugUtilsMessengerEXT dbg_messenger;

	uint32_t current_buffer;
	uint32_t queue_family_count;
};

//engine
#include "utils.h"
#include "engine.h"

static void demo_resize(struct demo *demo);

static bool memory_type_from_properties(struct demo *demo, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
		if ((typeBits & 1) == 1) {
			if ((demo->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

static void demo_flush_init_cmd(struct demo *demo) {
	VkResult U_ASSERT_ONLY err;

	if (demo->cmd == VK_NULL_HANDLE) return;

	err = vkEndCommandBuffer(demo->cmd);
	assert(!err);

	VkFence fence;
	VkFenceCreateInfo fence_ci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,.pNext = NULL,.flags = 0 };
	err = vkCreateFence(demo->device, &fence_ci, NULL, &fence);
	assert(!err);

	const VkCommandBuffer cmd_bufs[] = { demo->cmd };
	VkSubmitInfo submit_info = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = NULL,
		.pWaitDstStageMask = NULL,
		.commandBufferCount = 1,
		.pCommandBuffers = cmd_bufs,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = NULL };

	err = vkQueueSubmit(demo->graphics_queue, 1, &submit_info, fence);
	assert(!err);

	err = vkWaitForFences(demo->device, 1, &fence, VK_TRUE, UINT64_MAX);
	assert(!err);

	vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, cmd_bufs);
	vkDestroyFence(demo->device, fence, NULL);
	demo->cmd = VK_NULL_HANDLE;
}
static void demo_update_data_buffer(struct demo *demo, int id, float * p_pos);

void update_uniform_buffer(VkCommandBuffer cmd_buf, VkBuffer ub, void *data, uint32_t size)
{
	VkBufferMemoryBarrier ub_barrier = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.buffer = ub,
		.offset = 0,
		.size = size,
	};

	vkCmdPipelineBarrier(
		cmd_buf,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, NULL,
		1, &ub_barrier,
		0, NULL);

	vkCmdUpdateBuffer(cmd_buf, ub, 0, size, data);

	ub_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	ub_barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;

	vkCmdPipelineBarrier(
		cmd_buf,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, NULL,
		1, &ub_barrier,
		0, NULL);
}

static void demo_draw_build_cmd(struct demo *demo, VkCommandBuffer cmd_buf) {
	VkDebugUtilsLabelEXT label;
	memset(&label, 0, sizeof(label));
	const VkCommandBufferBeginInfo cmd_buf_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		.pInheritanceInfo = NULL,
	};
	const VkClearValue clear_values[2] = {
		[0] = { .color.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } },
		[1] = { .depthStencil = { 0.0f, 0 } },
	};
	const VkRenderPassBeginInfo rp_begin = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = demo->render_pass,
		.framebuffer = demo->swapchain_image_resources[demo->current_buffer].framebuffer,
		.renderArea.offset.x = 0,
		.renderArea.offset.y = 0,
		.renderArea.extent.width = demo->width,
		.renderArea.extent.height = demo->height,
		.clearValueCount = 2,
		.pClearValues = clear_values,
	};
	VkResult U_ASSERT_ONLY err;

	err = vkBeginCommandBuffer(cmd_buf, &cmd_buf_info);

	assert(!err);
	vkCmdBeginRenderPass(cmd_buf, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);


	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, demo->pipeline);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, demo->pipeline_layout, 0, 1,
		&demo->swapchain_image_resources[demo->current_buffer].descriptor_set, 0, NULL);
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.height = (float)demo->height;
	viewport.width = (float)demo->width;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(cmd_buf, 0, 1, &viewport);


	//background
	{
		float ta[2] = { 0,0 };
		int this_id = background;
		float ts[2];
		ts[0] = uniform_vals.obj_size[this_id][0];
		ts[1] = uniform_vals.obj_size[this_id][1];
		demo_update_data_buffer(demo, this_id, ta);
		VkRect2D scissor;
		memset(&scissor, 0, sizeof(scissor));
		scissor.extent.width = demo->width - (demo->width - ts[0] * demo->height);
		scissor.extent.height = demo->height - (demo->height - ts[1] * demo->height);
		scissor.offset.x = 0 + (demo->width / 2. - ts[0] / 2.*demo->height - ta[0] * demo->height);
		scissor.offset.y = 0 + (demo->height / 2. - ts[1] / 2.*demo->height + ta[1] * demo->height);
		vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

		vkCmdDraw(cmd_buf, 2 * 3, 1, 0, 0);
	}

	//debug
	if (drawdebug) {
		float ta[2] = { -0.7,-0.3 };
		//ta[0]=(float)uniform_vals.u_mouse[0]/demo->height;
		//ta[1]=1.-(float)uniform_vals.u_mouse[1]/demo->height;
		int this_id = debug;
		float ts[2];
		ts[0] = uniform_vals.obj_size[this_id][0];
		ts[1] = uniform_vals.obj_size[this_id][1];
		demo_update_data_buffer(demo, this_id, ta);
		VkRect2D scissor;
		memset(&scissor, 0, sizeof(scissor));
		scissor.extent.width = demo->width - (demo->width - ts[0] * demo->height);
		scissor.extent.height = demo->height - (demo->height - ts[1] * demo->height);
		scissor.offset.x = 0 + (demo->width / 2. - ts[0] / 2.*demo->height - ta[0] * demo->height);
		scissor.offset.y = 0 + (demo->height / 2. - ts[1] / 2.*demo->height + ta[1] * demo->height);
		vkCmdSetScissor(cmd_buf, 0, 1, &scissor);
		vkCmdDraw(cmd_buf, 2 * 3, 1, 0, 0);
	}

	//character
	{
		float ta[2] = { 0.56,0.25 };
		int this_id = character;
		float ts[2];
		ts[0] = uniform_vals.obj_size[this_id][0];
		ts[1] = uniform_vals.obj_size[this_id][1];
		demo_update_data_buffer(demo, this_id, ta);
		VkRect2D scissor;
		memset(&scissor, 0, sizeof(scissor));
		scissor.extent.width = demo->width - (demo->width - ts[0] * demo->height);
		scissor.extent.height = demo->height - (demo->height - ts[1] * demo->height);
		scissor.offset.x = 0 + (demo->width / 2. - ts[0] / 2.*demo->height - ta[0] * demo->height);
		scissor.offset.y = 0 + (demo->height / 2. - ts[1] / 2.*demo->height + ta[1] * demo->height);
		vkCmdSetScissor(cmd_buf, 0, 1, &scissor);
		vkCmdDraw(cmd_buf, 2 * 3, 1, 0, 0);
	}

	//particles
	for (int i = 0.; i<20; i++) {
		if (uniform_vals.p_isl[i])
		{
			float ta[2];
			ta[0] = uniform_vals.p_pos[i][0];
			ta[1] = uniform_vals.p_pos[i][1];
			int this_id = uniform_vals.p_id[i];
			float ts[2];
			ts[0] = uniform_vals.obj_size[this_id][0];
			ts[1] = uniform_vals.obj_size[this_id][1];
			demo_update_data_buffer(demo, this_id, ta);
			VkRect2D scissor;
			memset(&scissor, 0, sizeof(scissor));
			scissor.extent.width = demo->width - (demo->width - ts[0] * demo->height);
			scissor.extent.height = demo->height - (demo->height - ts[1] * demo->height);
			scissor.offset.x = 0 + (demo->width / 2. - ts[0] / 2.*demo->height - ta[0] * demo->height);
			scissor.offset.y = 0 + (demo->height / 2. - ts[1] / 2.*demo->height + ta[1] * demo->height);
			vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

			vkCmdDraw(cmd_buf, 2 * 3, 1, 0, 0);
		}
	}


	vkCmdEndRenderPass(cmd_buf);

	if (demo->separate_present_queue)
	{
		VkImageMemoryBarrier image_ownership_barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = NULL,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = demo->graphics_queue_family_index,
			.dstQueueFamilyIndex = demo->present_queue_family_index,
			.image = demo->swapchain_image_resources[demo->current_buffer].image,
			.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };

		vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
			NULL, 1, &image_ownership_barrier);
	}

	err = vkEndCommandBuffer(cmd_buf);
	assert(!err);
}

void demo_build_image_ownership_cmd(struct demo *demo, int i) {
	VkResult U_ASSERT_ONLY err;

	const VkCommandBufferBeginInfo cmd_buf_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		.pInheritanceInfo = NULL,
	};
	err = vkBeginCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd, &cmd_buf_info);
	assert(!err);

	VkImageMemoryBarrier image_ownership_barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = demo->graphics_queue_family_index,
		.dstQueueFamilyIndex = demo->present_queue_family_index,
		.image = demo->swapchain_image_resources[i].image,
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };

	vkCmdPipelineBarrier(demo->swapchain_image_resources[i].graphics_to_present_cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &image_ownership_barrier);
	err = vkEndCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd);
	assert(!err);
}

void demo_update_data_buffer(struct demo *demo, int id, float * p_pos) {
	uint8_t *pData;
	VkResult U_ASSERT_ONLY err;
	struct vktex_all_vs_uniform data;

	for (unsigned int i = 0; i < 2 * 3; i++) {
		data.position[i][0] = g_uv_buffer_data[2 * i];
		data.position[i][1] = g_uv_buffer_data[2 * i + 1];
		data.position[i][2] = 0.;
		data.position[i][3] = 1.;
	}
	data.u_resolution[0] = demo->width;
	data.u_resolution[1] = demo->height;
	//printf("%d \n",(int)data.u_resolution[1]);


	long ticks = get_time_ticks();
	data.u_time[0] = uniform_vals.u_time;
	data.u_time[1] = uniform_vals.FPS;
	data.draw_id[0] = id;
	data.draw_id[1] = id;
	data.draw_pos[0] = p_pos[0];
	data.draw_pos[1] = p_pos[1];
	data.u_mouse[0] = uniform_vals.u_mouse[0];
	data.u_mouse[1] = uniform_vals.u_mouse[1];
	data.u_hpdata[0] = uniform_vals.u_hpdata[0];
	data.u_hpdata[1] = uniform_vals.u_hpdata[1];
	data.u_hpdata[2] = uniform_vals.u_hpdata[2];
	data.u_hpdata[3] = demo->pause ? 10. : 0.;
	data.u_valt[0] = uniform_vals.u_valt[0];
	data.u_valt[1] = uniform_vals.u_valt[1];
	data.u_dataxx[0] = uniform_vals.u_dataxx[0];
	data.u_dataxx[1] = uniform_vals.u_dataxx[1];
	data.u_dataxx[2] = uniform_vals.u_dataxx[2];
	data.u_dataxx[3] = uniform_vals.u_dataxx[3];
	err = vkMapMemory(demo->device, demo->swapchain_image_resources[demo->current_buffer].uniform_memory, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
	assert(!err);

	memcpy(pData, &data, sizeof data);

	vkUnmapMemory(demo->device, demo->swapchain_image_resources[demo->current_buffer].uniform_memory);

	update_uniform_buffer(demo->swapchain_image_resources[demo->current_buffer].cmd, demo->swapchain_image_resources[demo->current_buffer].uniform_buffer, &data, sizeof data);
}

static void demo_draw(struct demo *demo) {
	VkResult U_ASSERT_ONLY err;
	uptate_my_vars(demo);
	// Ensure no more than FRAME_LAG renderings are outstanding
	vkWaitForFences(demo->device, 1, &demo->fences[demo->frame_index], VK_TRUE, UINT64_MAX);
	vkResetFences(demo->device, 1, &demo->fences[demo->frame_index]);

	do {
		err =
			demo->fpAcquireNextImageKHR(demo->device, demo->swapchain, UINT64_MAX,
				demo->image_acquired_semaphores[demo->frame_index], VK_NULL_HANDLE, &demo->current_buffer);

		if (err == VK_ERROR_OUT_OF_DATE_KHR) {
			demo_resize(demo);
		}
		else if (err == VK_SUBOPTIMAL_KHR) {
			break;
		}
		else {
			assert(!err);
		}
	} while (err != VK_SUCCESS);

	demo_draw_build_cmd(demo, demo->swapchain_image_resources[demo->current_buffer].cmd);

	VkPipelineStageFlags pipe_stage_flags;
	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.pWaitDstStageMask = &pipe_stage_flags;
	pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &demo->image_acquired_semaphores[demo->frame_index];
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &demo->swapchain_image_resources[demo->current_buffer].cmd;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
	err = vkQueueSubmit(demo->graphics_queue, 1, &submit_info, demo->fences[demo->frame_index]);
	assert(!err);

	if (demo->separate_present_queue) {
		VkFence nullFence = VK_NULL_HANDLE;
		pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &demo->swapchain_image_resources[demo->current_buffer].graphics_to_present_cmd;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &demo->image_ownership_semaphores[demo->frame_index];
		err = vkQueueSubmit(demo->present_queue, 1, &submit_info, nullFence);
		assert(!err);
	}

	VkPresentInfoKHR present = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = (demo->separate_present_queue) ? &demo->image_ownership_semaphores[demo->frame_index]
		: &demo->draw_complete_semaphores[demo->frame_index],
		.swapchainCount = 1,
		.pSwapchains = &demo->swapchain,
		.pImageIndices = &demo->current_buffer,
	};

	err = demo->fpQueuePresentKHR(demo->present_queue, &present);
	demo->frame_index += 1;
	demo->frame_index %= FRAME_LAG;

	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		demo_resize(demo);
	}
	else if (err == VK_SUBOPTIMAL_KHR) {
	}
	else {
		assert(!err);
	}
}

static void demo_prepare_buffers(struct demo *demo) {
	VkResult U_ASSERT_ONLY err;
	VkSwapchainKHR oldSwapchain = demo->swapchain;

	VkSurfaceCapabilitiesKHR surfCapabilities;
	err = demo->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(demo->gpu, demo->surface, &surfCapabilities);
	assert(!err);

	uint32_t presentModeCount;
	err = demo->fpGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &presentModeCount, NULL);
	assert(!err);
	VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	assert(presentModes);
	err = demo->fpGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &presentModeCount, presentModes);
	assert(!err);

	VkExtent2D swapchainExtent;
	if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
		swapchainExtent.width = demo->width;
		swapchainExtent.height = demo->height;

		if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
			swapchainExtent.width = surfCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
			swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}

		if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
			swapchainExtent.height = surfCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
			swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
	}
	else {
		swapchainExtent = surfCapabilities.currentExtent;
		demo->width = surfCapabilities.currentExtent.width;
		demo->height = surfCapabilities.currentExtent.height;
	}

	if (demo->width == 0 || demo->height == 0) {
		demo->is_minimized = true;
		return;
	}
	else {
		demo->is_minimized = false;
	}

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	//  There are times when you may wish to use another present mode.  The
	//  following code shows how to select them, and the comments provide some
	//  reasons you may wish to use them.
	//
	// It should be noted that Vulkan 1.0 doesn't provide a method for
	// synchronizing rendering with the presentation engine's display.  There
	// is a method provided for throttling rendering with the display, but
	// there are some presentation engines for which this method will not work.
	// If an application doesn't throttle its rendering, and if it renders much
	// faster than the refresh rate of the display, this can waste power on
	// mobile devices.  That is because power is being spent rendering images
	// that may never be seen.

	// VK_PRESENT_MODE_IMMEDIATE_KHR is for applications that don't care about
	// tearing, or have some way of synchronizing their rendering with the
	// display.
	// VK_PRESENT_MODE_MAILBOX_KHR may be useful for applications that
	// generally render a new presentable image every refresh cycle, but are
	// occasionally early.  In this case, the application wants the new image
	// to be displayed instead of the previously-queued-for-presentation image
	// that has not yet been displayed.
	// VK_PRESENT_MODE_FIFO_RELAXED_KHR is for applications that generally
	// render a new presentable image every refresh cycle, but are occasionally
	// late.  In this case (perhaps because of stuttering/latency concerns),
	// the application wants the late image to be immediately displayed, even
	// though that may mean some tearing.

	if (demo->presentMode != swapchainPresentMode) {
		for (size_t i = 0; i < presentModeCount; ++i) {
			if (presentModes[i] == demo->presentMode) {
				swapchainPresentMode = demo->presentMode;
				break;
			}
		}
	}
	if (swapchainPresentMode != demo->presentMode) {
		ERR_EXIT("Present mode specified is not supported, look --help \n", "Present mode unsupported");
	}

	uint32_t desiredNumOfSwapchainImages = 3;
	if (desiredNumOfSwapchainImages < surfCapabilities.minImageCount) {
		desiredNumOfSwapchainImages = surfCapabilities.minImageCount;
	}
	if ((surfCapabilities.maxImageCount > 0) && (desiredNumOfSwapchainImages > surfCapabilities.maxImageCount)) {
		desiredNumOfSwapchainImages = surfCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagsKHR preTransform;
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfCapabilities.currentTransform;
	}

	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (uint32_t i = 0; i < ARRAY_SIZE(compositeAlphaFlags); i++) {
		if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchain_ci = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.surface = demo->surface,
		.minImageCount = desiredNumOfSwapchainImages,
		.imageFormat = demo->format,
		.imageColorSpace = demo->color_space,
		.imageExtent =
	{
		.width = swapchainExtent.width,
		.height = swapchainExtent.height,
	},
	.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	.preTransform = preTransform,
	.compositeAlpha = compositeAlpha,
	.imageArrayLayers = 1,
	.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
	.queueFamilyIndexCount = 0,
	.pQueueFamilyIndices = NULL,
	.presentMode = swapchainPresentMode,
	.oldSwapchain = oldSwapchain,
	.clipped = true,
	};
	uint32_t i;
	err = demo->fpCreateSwapchainKHR(demo->device, &swapchain_ci, NULL, &demo->swapchain);
	assert(!err);

	if (oldSwapchain != VK_NULL_HANDLE) {
		demo->fpDestroySwapchainKHR(demo->device, oldSwapchain, NULL);
	}

	err = demo->fpGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchainImageCount, NULL);
	assert(!err);

	VkImage *swapchainImages = (VkImage *)malloc(demo->swapchainImageCount * sizeof(VkImage));
	assert(swapchainImages);
	err = demo->fpGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchainImageCount, swapchainImages);
	assert(!err);

	demo->swapchain_image_resources =
		(SwapchainImageResources *)malloc(sizeof(SwapchainImageResources) * demo->swapchainImageCount);
	assert(demo->swapchain_image_resources);

	for (i = 0; i < demo->swapchainImageCount; i++) {
		VkImageViewCreateInfo color_image_view = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.format = demo->format,
			.components =
		{
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		},
		.subresourceRange =
		{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,.baseMipLevel = 0,.levelCount = 1,.baseArrayLayer = 0,.layerCount = 1 },
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.flags = 0,
		};

		demo->swapchain_image_resources[i].image = swapchainImages[i];

		color_image_view.image = demo->swapchain_image_resources[i].image;

		err = vkCreateImageView(demo->device, &color_image_view, NULL, &demo->swapchain_image_resources[i].view);
		assert(!err);
	}

	if (NULL != presentModes) {
		free(presentModes);
	}
}

static void demo_prepare_depth(struct demo *demo) {
	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	const VkImageCreateInfo image = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depth_format,
		.extent = { demo->width, demo->height, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.flags = 0,
	};

	VkImageViewCreateInfo view = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.image = VK_NULL_HANDLE,
		.format = depth_format,
		.subresourceRange =
	{ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,.baseMipLevel = 0,.levelCount = 1,.baseArrayLayer = 0,.layerCount = 1 },
		.flags = 0,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
	};

	VkMemoryRequirements mem_reqs;
	VkResult U_ASSERT_ONLY err;
	bool U_ASSERT_ONLY pass;

	demo->depth.format = depth_format;

	/* create image */
	err = vkCreateImage(demo->device, &image, NULL, &demo->depth.image);
	assert(!err);

	vkGetImageMemoryRequirements(demo->device, demo->depth.image, &mem_reqs);
	assert(!err);

	demo->depth.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	demo->depth.mem_alloc.pNext = NULL;
	demo->depth.mem_alloc.allocationSize = mem_reqs.size;
	demo->depth.mem_alloc.memoryTypeIndex = 0;

	pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&demo->depth.mem_alloc.memoryTypeIndex);
	assert(pass);

	/* allocate memory */
	err = vkAllocateMemory(demo->device, &demo->depth.mem_alloc, NULL, &demo->depth.mem);
	assert(!err);

	/* bind memory */
	err = vkBindImageMemory(demo->device, demo->depth.image, demo->depth.mem, 0);
	assert(!err);

	/* create image view */
	view.image = demo->depth.image;
	err = vkCreateImageView(demo->device, &view, NULL, &demo->depth.view);
	assert(!err);
}

void demo_prepare_app_data_buffers(struct demo *demo) {
	VkBufferCreateInfo buf_info;
	VkMemoryRequirements mem_reqs;
	VkMemoryAllocateInfo mem_alloc;
	uint8_t *pData;
	VkResult U_ASSERT_ONLY err;
	bool U_ASSERT_ONLY pass;
	struct vktex_all_vs_uniform data;


	for (unsigned int i = 0; i < 2 * 3; i++) {
		data.position[i][0] = g_uv_buffer_data[2 * i];
		data.position[i][1] = g_uv_buffer_data[2 * i + 1];
		data.position[i][2] = 0.;
		data.position[i][3] = 1.;
	}
	data.u_resolution[0] = demo->width;
	data.u_resolution[1] = demo->height;
	long ticks = get_time_ticks();
	data.u_time[0] = 0.;
	data.u_time[1] = 0.;

	memset(&buf_info, 0, sizeof(buf_info));
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buf_info.size = sizeof(data);

	for (unsigned int i = 0; i < demo->swapchainImageCount; i++) {
		err = vkCreateBuffer(demo->device, &buf_info, NULL, &demo->swapchain_image_resources[i].uniform_buffer);
		assert(!err);

		vkGetBufferMemoryRequirements(demo->device, demo->swapchain_image_resources[i].uniform_buffer, &mem_reqs);

		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = mem_reqs.size;
		mem_alloc.memoryTypeIndex = 0;

		pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&mem_alloc.memoryTypeIndex);
		assert(pass);

		err = vkAllocateMemory(demo->device, &mem_alloc, NULL, &demo->swapchain_image_resources[i].uniform_memory);
		assert(!err);

		err = vkMapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
		assert(!err);

		memcpy(pData, &data, sizeof data);

		vkUnmapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory);

		err = vkBindBufferMemory(demo->device, demo->swapchain_image_resources[i].uniform_buffer,
			demo->swapchain_image_resources[i].uniform_memory, 0);
		assert(!err);
	}
}

static void demo_prepare_descriptor_layout(struct demo *demo) {
	const VkDescriptorSetLayoutBinding layout_bindings[1] = {
		[0] =
	{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = NULL,
	},
	};
	const VkDescriptorSetLayoutCreateInfo descriptor_layout = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.bindingCount = 1,
		.pBindings = layout_bindings,
	};
	VkResult U_ASSERT_ONLY err;

	err = vkCreateDescriptorSetLayout(demo->device, &descriptor_layout, NULL, &demo->desc_layout);
	assert(!err);

	const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.setLayoutCount = 1,
		.pSetLayouts = &demo->desc_layout,
	};

	err = vkCreatePipelineLayout(demo->device, &pPipelineLayoutCreateInfo, NULL, &demo->pipeline_layout);
	assert(!err);
}

static void demo_prepare_render_pass(struct demo *demo) {
	// The initial layout for the color and depth attachments will be LAYOUT_UNDEFINED
	// because at the start of the renderpass, we don't care about their contents.
	// At the start of the subpass, the color attachment's layout will be transitioned
	// to LAYOUT_COLOR_ATTACHMENT_OPTIMAL and the depth stencil attachment's layout
	// will be transitioned to LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.  At the end of
	// the renderpass, the color attachment's layout will be transitioned to
	// LAYOUT_PRESENT_SRC_KHR to be ready to present.  This is all done as part of
	// the renderpass, no barriers are necessary.
	const VkAttachmentDescription attachments[2] = {
		[0] =
	{
		.format = demo->format,
		.flags = 0,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	},
	[1] =
	{
		.format = demo->depth.format,
		.flags = 0,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	},
	};
	const VkAttachmentReference color_reference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	const VkAttachmentReference depth_reference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	const VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.flags = 0,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_reference,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = &depth_reference,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL,
	};
	const VkRenderPassCreateInfo rp_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = 2,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 0,
		.pDependencies = NULL,
	};
	VkResult U_ASSERT_ONLY err;

	err = vkCreateRenderPass(demo->device, &rp_info, NULL, &demo->render_pass);
	assert(!err);
}

static VkShaderModule demo_prepare_shader_module(struct demo *demo, const uint32_t *code, size_t size) {
	VkShaderModule module;
	VkShaderModuleCreateInfo moduleCreateInfo;
	VkResult U_ASSERT_ONLY err;

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = size;
	moduleCreateInfo.pCode = code;

	err = vkCreateShaderModule(demo->device, &moduleCreateInfo, NULL, &module);
	assert(!err);

	return module;
}

//#define SPIRV_SHADER

#ifndef SPIRV_SHADER
#include "yariv.h"
uint32_t *out_spirv;
size_t out_spirv_size;
bool yarivinit = false;
uint32_t *out_spirv_vert;
size_t out_spirv_size_vert;
bool yarivinit_vert = false;
#endif


static void demo_prepare_vs(struct demo *demo) {
	const unsigned char vs_code[] = {
#include "main.vert.inc"
	};
#ifdef SPIRV_SHADER
	demo->vert_shader_module = demo_prepare_shader_module(demo, (const uint32_t *)vs_code, sizeof(vs_code));
#else
	//YARIV
	if (!yarivinit_vert) {
		void *in_yariv = (void *)vs_code;
		size_t in_yariv_size = sizeof(vs_code);
		out_spirv_size_vert = yariv_decode_size(in_yariv, in_yariv_size);
		out_spirv_vert = malloc(out_spirv_size_vert);
		printf("unpack shader vert...\n");
		yariv_decode(out_spirv_vert, out_spirv_size_vert, in_yariv, in_yariv_size);
		printf("load shader vert...\n");
	}
	demo->vert_shader_module = demo_prepare_shader_module(demo, out_spirv_vert, out_spirv_size_vert); //-2 to fix uint8 size
	if (!yarivinit_vert)printf("shader loaded vert...\n");
	if (!yarivinit_vert)yarivinit_vert = true;
#endif
}

static void demo_prepare_fs(struct demo *demo) {
	const unsigned char fs_code[] = {
#include "main.frag.inc"
	};
	//SPIRV
#ifdef SPIRV_SHADER
	demo->frag_shader_module = demo_prepare_shader_module(demo, (const uint32_t *)fs_code, sizeof(fs_code));
#else
	//YARIV
	if (!yarivinit) {
		void *in_yariv = (void *)fs_code;
		size_t in_yariv_size = sizeof(fs_code);
		out_spirv_size = yariv_decode_size(in_yariv, in_yariv_size);
		out_spirv = malloc(out_spirv_size);
		printf("unpack shader frag...\n");
		yariv_decode(out_spirv, out_spirv_size, in_yariv, in_yariv_size);
		printf("load shader frag...\n");
	}
	demo->frag_shader_module = demo_prepare_shader_module(demo, out_spirv, out_spirv_size); //-2 to fix uint8 size
	if (!yarivinit)printf("shader loaded frag...\n");
	if (!yarivinit)yarivinit = true;
#endif
}

static void demo_prepare_pipeline(struct demo *demo) {
	VkGraphicsPipelineCreateInfo pipeline;
	VkPipelineCacheCreateInfo pipelineCache;
	VkPipelineVertexInputStateCreateInfo vi;
	VkPipelineInputAssemblyStateCreateInfo ia;
	VkPipelineRasterizationStateCreateInfo rs;
	VkPipelineColorBlendStateCreateInfo cb;
	VkPipelineDepthStencilStateCreateInfo ds;
	VkPipelineViewportStateCreateInfo vp;
	VkPipelineMultisampleStateCreateInfo ms;
	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	VkPipelineDynamicStateCreateInfo dynamicState;
	VkResult U_ASSERT_ONLY err;

	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
	memset(&dynamicState, 0, sizeof dynamicState);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables;

	memset(&pipeline, 0, sizeof(pipeline));
	pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline.layout = demo->pipeline_layout;

	memset(&vi, 0, sizeof(vi));
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	memset(&ia, 0, sizeof(ia));
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	memset(&rs, 0, sizeof(rs));
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = VK_CULL_MODE_BACK_BIT;
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.depthClampEnable = VK_FALSE;
	rs.rasterizerDiscardEnable = VK_FALSE;
	rs.depthBiasEnable = VK_FALSE;
	rs.lineWidth = 1.0f;

	memset(&cb, 0, sizeof(cb));
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState att_state[1];
	memset(att_state, 0, sizeof(att_state));
	att_state[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	att_state[0].blendEnable = VK_TRUE;
	att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
	att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
	cb.attachmentCount = 1;
	cb.pAttachments = att_state;

	memset(&vp, 0, sizeof(vp));
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount = 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	memset(&ds, 0, sizeof(ds));
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.back.failOp = VK_STENCIL_OP_KEEP;
	ds.back.passOp = VK_STENCIL_OP_KEEP;
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
	ds.stencilTestEnable = VK_FALSE;
	ds.front = ds.back;

	memset(&ms, 0, sizeof(ms));
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pSampleMask = NULL;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	demo_prepare_vs(demo);
	demo_prepare_fs(demo);

	// Two stages: vs and fs
	VkPipelineShaderStageCreateInfo shaderStages[2];
	memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = demo->vert_shader_module;
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = demo->frag_shader_module;
	shaderStages[1].pName = "main";

	memset(&pipelineCache, 0, sizeof(pipelineCache));
	pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	err = vkCreatePipelineCache(demo->device, &pipelineCache, NULL, &demo->pipelineCache);
	assert(!err);

	pipeline.pVertexInputState = &vi;
	pipeline.pInputAssemblyState = &ia;
	pipeline.pRasterizationState = &rs;
	pipeline.pColorBlendState = &cb;
	pipeline.pMultisampleState = &ms;
	pipeline.pViewportState = &vp;
	pipeline.pDepthStencilState = &ds;
	pipeline.stageCount = ARRAY_SIZE(shaderStages);
	pipeline.pStages = shaderStages;
	pipeline.renderPass = demo->render_pass;
	pipeline.pDynamicState = &dynamicState;

	pipeline.renderPass = demo->render_pass;

	err = vkCreateGraphicsPipelines(demo->device, demo->pipelineCache, 1, &pipeline, NULL, &demo->pipeline);
	assert(!err);

	vkDestroyShaderModule(demo->device, demo->frag_shader_module, NULL);
	vkDestroyShaderModule(demo->device, demo->vert_shader_module, NULL);
}

static void demo_prepare_descriptor_pool(struct demo *demo) {
	const VkDescriptorPoolSize type_counts[1] = {
		[0] =
	{
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = demo->swapchainImageCount,
	},
	};
	const VkDescriptorPoolCreateInfo descriptor_pool = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.maxSets = demo->swapchainImageCount,
		.poolSizeCount = 1,
		.pPoolSizes = type_counts,
	};
	VkResult U_ASSERT_ONLY err;

	err = vkCreateDescriptorPool(demo->device, &descriptor_pool, NULL, &demo->desc_pool);
	assert(!err);
}

static void demo_prepare_descriptor_set(struct demo *demo) {
	VkWriteDescriptorSet writes[1];
	VkResult U_ASSERT_ONLY err;

	VkDescriptorSetAllocateInfo alloc_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = demo->desc_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &demo->desc_layout };

	VkDescriptorBufferInfo buffer_info;
	buffer_info.offset = 0;
	buffer_info.range = sizeof(struct vktex_all_vs_uniform);

	memset(&writes, 0, sizeof(writes));

	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &buffer_info;

	for (unsigned int i = 0; i < demo->swapchainImageCount; i++) {
		err = vkAllocateDescriptorSets(demo->device, &alloc_info, &demo->swapchain_image_resources[i].descriptor_set);
		assert(!err);
		buffer_info.buffer = demo->swapchain_image_resources[i].uniform_buffer;
		writes[0].dstSet = demo->swapchain_image_resources[i].descriptor_set;
		vkUpdateDescriptorSets(demo->device, 1, writes, 0, NULL);
	}
}

static void demo_prepare_framebuffers(struct demo *demo) {
	VkImageView attachments[2];
	attachments[1] = demo->depth.view;

	const VkFramebufferCreateInfo fb_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = NULL,
		.renderPass = demo->render_pass,
		.attachmentCount = 2,
		.pAttachments = attachments,
		.width = demo->width,
		.height = demo->height,
		.layers = 1,
	};
	VkResult U_ASSERT_ONLY err;
	uint32_t i;

	for (i = 0; i < demo->swapchainImageCount; i++) {
		attachments[0] = demo->swapchain_image_resources[i].view;
		err = vkCreateFramebuffer(demo->device, &fb_info, NULL, &demo->swapchain_image_resources[i].framebuffer);
		assert(!err);
	}
}

static void demo_prepare(struct demo *demo) {
	VkResult U_ASSERT_ONLY err;
	if (demo->cmd_pool == VK_NULL_HANDLE) {
		const VkCommandPoolCreateInfo cmd_pool_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.queueFamilyIndex = demo->graphics_queue_family_index,
			.flags = 0,
		};
		err = vkCreateCommandPool(demo->device, &cmd_pool_info, NULL, &demo->cmd_pool);
		assert(!err);
	}

	const VkCommandBufferAllocateInfo cmd = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = demo->cmd_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->cmd);
	assert(!err);
	VkCommandBufferBeginInfo cmd_buf_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};
	err = vkBeginCommandBuffer(demo->cmd, &cmd_buf_info);
	assert(!err);

	demo_prepare_buffers(demo);

	if (demo->is_minimized) {
		demo->prepared = false;
		return;
	}

	demo_prepare_depth(demo);
	demo_prepare_app_data_buffers(demo);

	demo_prepare_descriptor_layout(demo);
	demo_prepare_render_pass(demo);
	demo_prepare_pipeline(demo);

	for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
		err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->swapchain_image_resources[i].cmd);
		assert(!err);
	}

	if (demo->separate_present_queue) {
		const VkCommandPoolCreateInfo present_cmd_pool_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.queueFamilyIndex = demo->present_queue_family_index,
			.flags = 0,
		};
		err = vkCreateCommandPool(demo->device, &present_cmd_pool_info, NULL, &demo->present_cmd_pool);
		assert(!err);
		const VkCommandBufferAllocateInfo present_cmd_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = demo->present_cmd_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
			err = vkAllocateCommandBuffers(demo->device, &present_cmd_info,
				&demo->swapchain_image_resources[i].graphics_to_present_cmd);
			assert(!err);
			demo_build_image_ownership_cmd(demo, i);
		}
	}

	demo_prepare_descriptor_pool(demo);
	demo_prepare_descriptor_set(demo);

	demo_prepare_framebuffers(demo);

	for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
		demo->current_buffer = i;
		demo_draw_build_cmd(demo, demo->swapchain_image_resources[i].cmd);
	}

	/*
	* Prepare functions above may generate pipeline commands
	* that need to be flushed before beginning the render loop.
	*/
	demo_flush_init_cmd(demo);

	demo->current_buffer = 0;
	demo->prepared = true;
}

static void demo_cleanup(struct demo *demo) {
	uint32_t i;

	demo->prepared = false;
	vkDeviceWaitIdle(demo->device);

	// Wait for fences from present operations
	for (i = 0; i < FRAME_LAG; i++) {
		vkWaitForFences(demo->device, 1, &demo->fences[i], VK_TRUE, UINT64_MAX);
		vkDestroyFence(demo->device, demo->fences[i], NULL);
		vkDestroySemaphore(demo->device, demo->image_acquired_semaphores[i], NULL);
		vkDestroySemaphore(demo->device, demo->draw_complete_semaphores[i], NULL);
		if (demo->separate_present_queue) {
			vkDestroySemaphore(demo->device, demo->image_ownership_semaphores[i], NULL);
		}
	}

	// If the window is currently minimized, demo_resize has already done some cleanup for us.
	if (!demo->is_minimized) {
		for (i = 0; i < demo->swapchainImageCount; i++) {
			vkDestroyFramebuffer(demo->device, demo->swapchain_image_resources[i].framebuffer, NULL);
		}
		vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

		vkDestroyPipeline(demo->device, demo->pipeline, NULL);
		vkDestroyPipelineCache(demo->device, demo->pipelineCache, NULL);
		vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
		vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
		vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

		demo->fpDestroySwapchainKHR(demo->device, demo->swapchain, NULL);

		vkDestroyImageView(demo->device, demo->depth.view, NULL);
		vkDestroyImage(demo->device, demo->depth.image, NULL);
		vkFreeMemory(demo->device, demo->depth.mem, NULL);

		for (i = 0; i < demo->swapchainImageCount; i++) {
			vkDestroyImageView(demo->device, demo->swapchain_image_resources[i].view, NULL);
			vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->swapchain_image_resources[i].cmd);
			vkDestroyBuffer(demo->device, demo->swapchain_image_resources[i].uniform_buffer, NULL);
			vkFreeMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, NULL);
		}
		free(demo->swapchain_image_resources);
		free(demo->queue_props);
		vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);

		if (demo->separate_present_queue) {
			vkDestroyCommandPool(demo->device, demo->present_cmd_pool, NULL);
		}
	}
	vkDeviceWaitIdle(demo->device);
	vkDestroyDevice(demo->device, NULL);

	vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);

#if defined(VK_USE_PLATFORM_XCB_KHR)
	xcb_destroy_window(demo->connection, demo->xcb_window);
	xcb_disconnect(demo->connection);
	free(demo->atom_wm_delete_window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	wl_keyboard_destroy(demo->keyboard);
	wl_pointer_destroy(demo->pointer);
	wl_seat_destroy(demo->seat);
	wl_shell_surface_destroy(demo->shell_surface);
	wl_surface_destroy(demo->window);
	wl_shell_destroy(demo->shell);
	wl_compositor_destroy(demo->compositor);
	wl_registry_destroy(demo->registry);
	wl_display_disconnect(demo->display);
#endif

	vkDestroyInstance(demo->inst, NULL);
}

static void demo_resize(struct demo *demo) {
	uint32_t i;

	// Don't react to resize until after first initialization.
	if (!demo->prepared) {
		if (demo->is_minimized) {
			demo_prepare(demo);
		}
		return;
	}
	// In order to properly resize the window, we must re-create the swapchain
	// AND redo the command buffers, etc.
	//
	// First, perform part of the demo_cleanup() function:
	demo->prepared = false;
	vkDeviceWaitIdle(demo->device);

	for (i = 0; i < demo->swapchainImageCount; i++) {
		vkDestroyFramebuffer(demo->device, demo->swapchain_image_resources[i].framebuffer, NULL);
	}
	vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

	vkDestroyPipeline(demo->device, demo->pipeline, NULL);
	vkDestroyPipelineCache(demo->device, demo->pipelineCache, NULL);
	vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
	vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
	vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

	vkDestroyImageView(demo->device, demo->depth.view, NULL);
	vkDestroyImage(demo->device, demo->depth.image, NULL);
	vkFreeMemory(demo->device, demo->depth.mem, NULL);

	for (i = 0; i < demo->swapchainImageCount; i++) {
		vkDestroyImageView(demo->device, demo->swapchain_image_resources[i].view, NULL);
		vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->swapchain_image_resources[i].cmd);
		vkDestroyBuffer(demo->device, demo->swapchain_image_resources[i].uniform_buffer, NULL);
		vkFreeMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, NULL);
	}
	vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);
	demo->cmd_pool = VK_NULL_HANDLE;
	if (demo->separate_present_queue) {
		vkDestroyCommandPool(demo->device, demo->present_cmd_pool, NULL);
	}
	free(demo->swapchain_image_resources);

	// Second, re-perform the demo_prepare() function, which will re-create the
	// swapchain:
	demo_prepare(demo);
}

// On MS-Windows, make this a global, so it's available to WndProc()
struct demo demo;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void demo_run(struct demo *demo) {
	if (!demo->prepared) return;

	demo_draw(demo);
	demo->curFrame++;
	if (demo->frameCount != INT_MAX && demo->curFrame == demo->frameCount) {
		PostQuitMessage(validation_error);
	}
}

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(validation_error);
		break;
	case WM_PAINT:
		// The validation callback calls MessageBox which can generate paint
		// events - don't make more Vulkan calls if we got here from the
		// callback
		if (!in_callback) {
			demo_run(&demo);
		}
		break;
	case WM_GETMINMAXINFO:  // set window's minimum size
		((MINMAXINFO *)lParam)->ptMinTrackSize = demo.minsize;
		return 0;
	case WM_SIZE:
		// Resize the application to the new window size, except when
		// it was minimized. Vulkan doesn't support images or swapchains
		// with width=0 and height=0.
		if (wParam != SIZE_MINIMIZED) {
			demo.width = lParam & 0xffff;
			demo.height = (lParam & 0xffff0000) >> 16;
			demo_resize(&demo);
		}
		break;
	default:
		break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

static void demo_create_window(struct demo *demo) {
	WNDCLASSEX win_class;

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WndProc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = demo->connection;  // hInstance
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = demo->name;
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassEx(&win_class)) {
		// It didn't work, so try to give a useful error:
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}
	// Create window with the registered class:
	RECT wr = { 0, 0, demo->width, demo->height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	demo->window = CreateWindowEx(0,
		demo->name,            // class name
		demo->name,            // app name
		WS_OVERLAPPEDWINDOW |  // window style
		WS_VISIBLE | WS_SYSMENU,
		100, 100,            // x/y coords
		wr.right - wr.left,  // width
		wr.bottom - wr.top,  // height
		NULL,                // handle to parent
		NULL,                // handle to menu
		demo->connection,    // hInstance
		NULL);               // no extra parameters
	if (!demo->window) {
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}
	// Window client area size must be at least 1 pixel high, to prevent crash.
	demo->minsize.x = GetSystemMetrics(SM_CXMINTRACK);
	demo->minsize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;
}
#elif defined(VK_USE_PLATFORM_XCB_KHR)

static void print_modifiers(uint32_t mask)
{
	const char **mod, *mods[] = {
		"Shift", "Lock", "Ctrl", "Alt",
		"Mod2", "Mod3", "Mod4", "Mod5",
		"Button1", "Button2", "Button3", "Button4", "Button5"
	};
	printf("Modifier mask: ");
	for (mod = mods; mask; mask >>= 1, mod++)
		if (mask & 1)
			printf(*mod);
	putchar('\n');
}

static void demo_handle_xcb_event(struct demo *demo, const xcb_generic_event_t *event) {
	uint8_t event_code = event->response_type & 0x7f;
	switch (event_code) {
	case XCB_EXPOSE:
		break;
	case XCB_CLIENT_MESSAGE:
		if ((*(xcb_client_message_event_t *)event).data.data32[0] == (*demo->atom_wm_delete_window).atom) {
			demo->quit = true;
		}
		break;
	case XCB_KEY_RELEASE: {
		const xcb_key_release_event_t *key = (const xcb_key_release_event_t *)event;
		switch (key->detail) {
		case 0x9:  // Escape
			demo->quit = true;
			break;
		case 0x71:  // left arrow key
			demo->spin_angle -= demo->spin_increment;
			break;
		case 0x72:  // right arrow key
			demo->spin_angle += demo->spin_increment;
			break;
		case 0x41:  // space bar
			demo->pause = !demo->pause;
			pres_pause(demo);
			break;
		case 0xa: //1
			drawdebug = !drawdebug;
			break;
		}
	} break;
	case XCB_MOTION_NOTIFY: {
		const xcb_motion_notify_event_t *ev = (const xcb_motion_notify_event_t *)event;
		//printf ("Mouse moved in window %ld, at coordinates (%d,%d)\n",ev->event, ev->event_x, ev->event_y);
		uniform_vals.u_mouse[0] = ev->event_x;
		uniform_vals.u_mouse[1] = demo->height - ev->event_y;
	} break;
	case XCB_BUTTON_PRESS: {
		const xcb_button_press_event_t *ev = (const xcb_button_press_event_t *)event;
		//print_modifiers(ev->state);
		switch (ev->detail) {
		case 4:
			/*printf ("Wheel Button up in window %ld, at coordinates (%d,%d)\n",
			ev->event, ev->event_x, ev->event_y);*/
			break;
		case 5:
			/*printf ("Wheel Button down in window %ld, at coordinates (%d,%d)\n",
			ev->event, ev->event_x, ev->event_y);*/
			break;
		default:
			/*printf ("Button %d pressed in window %ld, at coordinates (%d,%d)\n",
			ev->detail, ev->event, ev->event_x, ev->event_y);*/
			if (ev->detail == 1)if (!m_left_c)m_left = true;
			if (ev->detail == 3)if (!m_right_c)m_right = true;
		}
	} break;
	case XCB_BUTTON_RELEASE: {
		const xcb_button_release_event_t *ev = (const xcb_button_release_event_t *)event;
		if (ev->detail == 1)m_left = false;
		if (ev->detail == 3)m_right = false;
		/*print_modifiers(ev->state);

		printf ("Button %d released in window %ld, at coordinates (%d,%d)\n",
		ev->detail, ev->event, ev->event_x, ev->event_y);*/
	} break;

	case XCB_CONFIGURE_NOTIFY: {
		const xcb_configure_notify_event_t *cfg = (const xcb_configure_notify_event_t *)event;
		if ((demo->width != cfg->width) || (demo->height != cfg->height)) {
			demo->width = cfg->width;
			demo->height = cfg->height;
			demo_resize(demo);
		}
	} break;
	default:
		break;
	}
}

#ifdef USE_PTHREAD

#include <pthread.h>

struct thread_args {
	xcb_generic_event_t *event;
	struct demo *demo;
};

void *event_thread(void *event_void_ptr)
{
	struct thread_args *ta = event_void_ptr;
	ta->event = xcb_wait_for_event(ta->demo->connection);
	return NULL;
}

static void demo_run_xcb(struct demo *demo) {
	xcb_flush(demo->connection);
	struct thread_args args;
	args.demo = demo;
	args.event = xcb_poll_for_event(demo->connection);
	pthread_t event_thread_id = 0;
	while (!demo->quit) {
		if (event_thread_id == 0) {
			if (pthread_create(&event_thread_id, NULL, event_thread, &args)) {
				fprintf(stderr, "Error creating thread\n");
				exit(1);
			}
		}
		if (pthread_kill(event_thread_id, 0) == 0) {}
		else
		{
			//event
			pthread_join(event_thread_id, NULL);
			event_thread_id = 0;
			while (args.event) {
				demo_handle_xcb_event(demo, args.event);
				free(args.event);
				args.event = xcb_poll_for_event(demo->connection);
			}
		}

		demo_draw(demo);
		demo->curFrame++;
		if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) demo->quit = true;
	}
	if (pthread_kill(event_thread_id, 0) == 0) {
		pthread_join(event_thread_id, NULL);
	}
}

#else

static void demo_run_xcb(struct demo *demo) {
	xcb_flush(demo->connection);
	while (!demo->quit) {
		xcb_generic_event_t *event;

		if (demo->pause) {
			event = xcb_wait_for_event(demo->connection);
		}
		else {
			event = xcb_poll_for_event(demo->connection);
		}
		while (event) {
			demo_handle_xcb_event(demo, event);
			free(event);
			event = xcb_poll_for_event(demo->connection);
		}

		demo_draw(demo);
		demo->curFrame++;
		if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) demo->quit = true;
	}
}

#endif


static void demo_create_xcb_window(struct demo *demo) {
	uint32_t value_mask, value_list[32];

	demo->xcb_window = xcb_generate_id(demo->connection);

	value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	value_list[0] = demo->screen->black_pixel;
	value_list[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

	xcb_create_window(demo->connection, XCB_COPY_FROM_PARENT, demo->xcb_window, demo->screen->root, 0, 0, demo->width, demo->height,
		0, XCB_WINDOW_CLASS_INPUT_OUTPUT, demo->screen->root_visual, value_mask, value_list);

	/* Magic code that will send notification when window is destroyed */
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(demo->connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(demo->connection, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(demo->connection, 0, 16, "WM_DELETE_WINDOW");
	demo->atom_wm_delete_window = xcb_intern_atom_reply(demo->connection, cookie2, 0);

	xcb_change_property(demo->connection, XCB_PROP_MODE_REPLACE, demo->xcb_window, (*reply).atom, 4, 32, 1,
		&(*demo->atom_wm_delete_window).atom);
	free(reply);

	xcb_map_window(demo->connection, demo->xcb_window);

	// Force the x/y coordinates to 100,100 results are identical in consecutive
	// runs
	const uint32_t coords[] = { 100, 100 };
	xcb_configure_window(demo->connection, demo->xcb_window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
}
// VK_USE_PLATFORM_XCB_KHR
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
static void demo_run(struct demo *demo) {
	while (!demo->quit) {
		if (demo->pause) {
			wl_display_dispatch(demo->display);  // block and wait for input
		}
		else {
			wl_display_dispatch_pending(demo->display);  // don't block
			demo_draw(demo);
			demo->curFrame++;
			if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) demo->quit = true;
		}
	}
}

static void handle_ping(void *data UNUSED, struct wl_shell_surface *shell_surface, uint32_t serial) {
	wl_shell_surface_pong(shell_surface, serial);
}

static void handle_configure(void *data UNUSED, struct wl_shell_surface *shell_surface UNUSED, uint32_t edges UNUSED,
	int32_t width UNUSED, int32_t height UNUSED) {}

static void handle_popup_done(void *data UNUSED, struct wl_shell_surface *shell_surface UNUSED) {}

static const struct wl_shell_surface_listener shell_surface_listener = { handle_ping, handle_configure, handle_popup_done };

static void demo_create_window(struct demo *demo) {
	demo->window = wl_compositor_create_surface(demo->compositor);
	if (!demo->window) {
		printf("Can not create wayland_surface from compositor!\n");
		fflush(stdout);
		exit(1);
	}

	demo->shell_surface = wl_shell_get_shell_surface(demo->shell, demo->window);
	if (!demo->shell_surface) {
		printf("Can not get shell_surface from wayland_surface!\n");
		fflush(stdout);
		exit(1);
	}
	wl_shell_surface_add_listener(demo->shell_surface, &shell_surface_listener, demo);
	wl_shell_surface_set_toplevel(demo->shell_surface);
	wl_shell_surface_set_title(demo->shell_surface, APP_SHORT_NAME);
}
#endif

static void demo_init_vk(struct demo *demo) {
	VkResult err;
	uint32_t instance_extension_count = 0;


	demo->enabled_extension_count = 0;
	demo->enabled_layer_count = 0;
	demo->is_minimized = false;
	demo->cmd_pool = VK_NULL_HANDLE;

	/* Look for instance extensions */
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;
	memset(demo->extension_names, 0, sizeof(demo->extension_names));

	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	assert(!err);

	if (instance_extension_count > 0) {
		VkExtensionProperties *instance_extensions = malloc(sizeof(VkExtensionProperties) * instance_extension_count);
		err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
		assert(!err);
		for (uint32_t i = 0; i < instance_extension_count; i++) {
			if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				surfaceExtFound = 1;
				demo->extension_names[demo->enabled_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				platformSurfaceExtFound = 1;
				demo->extension_names[demo->enabled_extension_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
			if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				platformSurfaceExtFound = 1;
				demo->extension_names[demo->enabled_extension_count++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
			}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
			if (!strcmp(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				platformSurfaceExtFound = 1;
				demo->extension_names[demo->enabled_extension_count++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
			}
#endif
			if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instance_extensions[i].extensionName)) {

			}
			assert(demo->enabled_extension_count < 64);
		}

		free(instance_extensions);
	}

	if (!surfaceExtFound) {
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME
			" extension.\n\n"
			"Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
	}
	if (!platformSurfaceExtFound) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME
			" extension.\n\n"
			"Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_XCB_SURFACE_EXTENSION_NAME
			" extension.\n\n"
			"Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
			" extension.\n\n"
			"Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
#endif
	}
	const VkApplicationInfo app = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = APP_SHORT_NAME,
		.applicationVersion = 0,
		.pEngineName = APP_SHORT_NAME,
		.engineVersion = 0,
		.apiVersion = VK_API_VERSION_1_0,
	};
	VkInstanceCreateInfo inst_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.pApplicationInfo = &app,
		.enabledLayerCount = demo->enabled_layer_count,
		.enabledExtensionCount = demo->enabled_extension_count,
		.ppEnabledExtensionNames = (const char *const *)demo->extension_names,
	};


	uint32_t gpu_count;

	err = vkCreateInstance(&inst_info, NULL, &demo->inst);

	if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
		ERR_EXIT(
			"Cannot find a compatible Vulkan installable client driver (ICD).\n\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
	}
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
		ERR_EXIT(
			"Cannot find a specified extension library.\n"
			"Make sure your layers path is set appropriately.\n",
			"vkCreateInstance Failure");
	}
	else if (err) {
		ERR_EXIT(
			"vkCreateInstance failed.\n\n"
			"Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
	}

	err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count, NULL);
	assert(!err);

	if (gpu_count > 0) {
		VkPhysicalDevice *physical_devices = malloc(sizeof(VkPhysicalDevice) * gpu_count);
		err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count, physical_devices);
		assert(!err);
		demo->gpu = physical_devices[0];
		free(physical_devices);
	}
	else {
		ERR_EXIT(
			"vkEnumeratePhysicalDevices reported zero accessible devices.\n\n"
			"Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkEnumeratePhysicalDevices Failure");
	}

	/* Look for device extensions */
	uint32_t device_extension_count = 0;
	VkBool32 swapchainExtFound = 0;
	demo->enabled_extension_count = 0;
	memset(demo->extension_names, 0, sizeof(demo->extension_names));

	err = vkEnumerateDeviceExtensionProperties(demo->gpu, NULL, &device_extension_count, NULL);
	assert(!err);

	if (device_extension_count > 0) {
		VkExtensionProperties *device_extensions = malloc(sizeof(VkExtensionProperties) * device_extension_count);
		err = vkEnumerateDeviceExtensionProperties(demo->gpu, NULL, &device_extension_count, device_extensions);
		assert(!err);

		for (uint32_t i = 0; i < device_extension_count; i++) {
			if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName)) {
				swapchainExtFound = 1;
				demo->extension_names[demo->enabled_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			}
			assert(demo->enabled_extension_count < 64);
		}

		free(device_extensions);
	}

	if (!swapchainExtFound) {
		ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
			" extension.\n\nDo you have a compatible Vulkan installable client driver (ICD) installed?\n"
			"Please look at the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
	}

	vkGetPhysicalDeviceProperties(demo->gpu, &demo->gpu_props);

	/* Call with NULL data to get count */
	vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_family_count, NULL);
	assert(demo->queue_family_count >= 1);

	demo->queue_props = (VkQueueFamilyProperties *)malloc(demo->queue_family_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_family_count, demo->queue_props);

	// Query fine-grained feature support for this device.
	//  If app has specific feature requirements it should check supported
	//  features based on this query
	VkPhysicalDeviceFeatures physDevFeatures;
	vkGetPhysicalDeviceFeatures(demo->gpu, &physDevFeatures);

	GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(demo->inst, GetSwapchainImagesKHR);
}

static void demo_create_device(struct demo *demo) {
	VkResult U_ASSERT_ONLY err;
	float queue_priorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queues[2];
	queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[0].pNext = NULL;
	queues[0].queueFamilyIndex = demo->graphics_queue_family_index;
	queues[0].queueCount = 1;
	queues[0].pQueuePriorities = queue_priorities;
	queues[0].flags = 0;

	VkDeviceCreateInfo device = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = queues,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = demo->enabled_extension_count,
		.ppEnabledExtensionNames = (const char *const *)demo->extension_names,
		.pEnabledFeatures = NULL,  // If specific features are required, pass them in here
	};
	if (demo->separate_present_queue) {
		queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queues[1].pNext = NULL;
		queues[1].queueFamilyIndex = demo->present_queue_family_index;
		queues[1].queueCount = 1;
		queues[1].pQueuePriorities = queue_priorities;
		queues[1].flags = 0;
		device.queueCreateInfoCount = 2;
	}
	err = vkCreateDevice(demo->gpu, &device, NULL, &demo->device);
	assert(!err);
}

static void demo_init_vk_swapchain(struct demo *demo) {
	VkResult U_ASSERT_ONLY err;

	// Create a WSI surface for the window:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.hinstance = demo->connection;
	createInfo.hwnd = demo->window;

	err = vkCreateWin32SurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	VkWaylandSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.display = demo->display;
	createInfo.surface = demo->window;

	err = vkCreateWaylandSurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VkXcbSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.connection = demo->connection;
	createInfo.window = demo->xcb_window;

	err = vkCreateXcbSurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#endif
	assert(!err);

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *supportsPresent = (VkBool32 *)malloc(demo->queue_family_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < demo->queue_family_count; i++) {
		demo->fpGetPhysicalDeviceSurfaceSupportKHR(demo->gpu, i, demo->surface, &supportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t presentQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < demo->queue_family_count; i++) {
		if ((demo->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (graphicsQueueFamilyIndex == UINT32_MAX) {
				graphicsQueueFamilyIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueFamilyIndex = i;
				presentQueueFamilyIndex = i;
				break;
			}
		}
	}

	if (presentQueueFamilyIndex == UINT32_MAX) {
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (uint32_t i = 0; i < demo->queue_family_count; ++i) {
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueFamilyIndex = i;
				break;
			}
		}
	}

	// Generate error if could not find both a graphics and a present queue
	if (graphicsQueueFamilyIndex == UINT32_MAX || presentQueueFamilyIndex == UINT32_MAX) {
		ERR_EXIT("Could not find both graphics and present queues\n", "Swapchain Initialization Failure");
	}

	demo->graphics_queue_family_index = graphicsQueueFamilyIndex;
	demo->present_queue_family_index = presentQueueFamilyIndex;
	demo->separate_present_queue = (demo->graphics_queue_family_index != demo->present_queue_family_index);
	free(supportsPresent);

	demo_create_device(demo);

	GET_DEVICE_PROC_ADDR(demo->device, CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(demo->device, DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(demo->device, GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(demo->device, AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(demo->device, QueuePresentKHR);

	vkGetDeviceQueue(demo->device, demo->graphics_queue_family_index, 0, &demo->graphics_queue);

	if (!demo->separate_present_queue) {
		demo->present_queue = demo->graphics_queue;
	}
	else {
		vkGetDeviceQueue(demo->device, demo->present_queue_family_index, 0, &demo->present_queue);
	}

	// Get the list of VkFormat's that are supported:
	uint32_t formatCount;
	err = demo->fpGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface, &formatCount, NULL);
	assert(!err);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	err = demo->fpGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface, &formatCount, surfFormats);
	assert(!err);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		demo->format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		assert(formatCount >= 1);
		demo->format = surfFormats[0].format;
	}
	demo->color_space = surfFormats[0].colorSpace;

	demo->quit = false;
	demo->curFrame = 0;

	// Create semaphores to synchronize acquiring presentable buffers before
	// rendering and waiting for drawing to be complete before presenting
	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};

	// Create fences that we can use to throttle if we get too far
	// ahead of the image presents
	VkFenceCreateInfo fence_ci = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,.pNext = NULL,.flags = VK_FENCE_CREATE_SIGNALED_BIT };
	for (uint32_t i = 0; i < FRAME_LAG; i++) {
		err = vkCreateFence(demo->device, &fence_ci, NULL, &demo->fences[i]);
		assert(!err);

		err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->image_acquired_semaphores[i]);
		assert(!err);

		err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->draw_complete_semaphores[i]);
		assert(!err);

		if (demo->separate_present_queue) {
			err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->image_ownership_semaphores[i]);
			assert(!err);
		}
	}
	demo->frame_index = 0;

	// Get Memory information and properties
	vkGetPhysicalDeviceMemoryProperties(demo->gpu, &demo->memory_properties);
}

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx,
	wl_fixed_t sy) {}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button,
	uint32_t state) {
	struct demo *demo = data;
	if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
		wl_shell_surface_move(demo->shell_surface, demo->seat, serial);
	}
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {}

static const struct wl_pointer_listener pointer_listener = {
	pointer_handle_enter, pointer_handle_leave, pointer_handle_motion, pointer_handle_button, pointer_handle_axis,
};

static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size) {}

static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface,
	struct wl_array *keys) {}

static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {}

static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key,
	uint32_t state) {
	if (state != WL_KEYBOARD_KEY_STATE_RELEASED) return;
	struct demo *demo = data;
	switch (key) {
	case KEY_ESC:  // Escape
		demo->quit = true;
		break;
	case KEY_LEFT:  // left arrow key
		demo->spin_angle -= demo->spin_increment;
		break;
	case KEY_RIGHT:  // right arrow key
		demo->spin_angle += demo->spin_increment;
		break;
	case KEY_SPACE:  // space bar
		demo->pause = !demo->pause;
		break;
	}
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
	uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {}

static const struct wl_keyboard_listener keyboard_listener = {
	keyboard_handle_keymap, keyboard_handle_enter, keyboard_handle_leave, keyboard_handle_key, keyboard_handle_modifiers,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps) {
	// Subscribe to pointer events
	struct demo *demo = data;
	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !demo->pointer) {
		demo->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(demo->pointer, &pointer_listener, demo);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && demo->pointer) {
		wl_pointer_destroy(demo->pointer);
		demo->pointer = NULL;
	}
	// Subscribe to keyboard events
	if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
		demo->keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(demo->keyboard, &keyboard_listener, demo);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
		wl_keyboard_destroy(demo->keyboard);
		demo->keyboard = NULL;
	}
}

static const struct wl_seat_listener seat_listener = {
	seat_handle_capabilities,
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
	uint32_t version UNUSED) {
	struct demo *demo = data;
	// pickup wayland objects when they appear
	if (strcmp(interface, "wl_compositor") == 0) {
		demo->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
	}
	else if (strcmp(interface, "wl_shell") == 0) {
		demo->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
	}
	else if (strcmp(interface, "wl_seat") == 0) {
		demo->seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
		wl_seat_add_listener(demo->seat, &seat_listener, demo);
	}
}

static void registry_handle_global_remove(void *data UNUSED, struct wl_registry *registry UNUSED, uint32_t name UNUSED) {}

static const struct wl_registry_listener registry_listener = { registry_handle_global, registry_handle_global_remove };
#endif

static void demo_init_connection(struct demo *demo) {
#if defined(VK_USE_PLATFORM_XCB_KHR)
	const xcb_setup_t *setup;
	xcb_screen_iterator_t iter;
	int scr;

	const char *display_envar = getenv("DISPLAY");
	if (display_envar == NULL || display_envar[0] == '\0') {
		printf("Environment variable DISPLAY requires a valid value.\nExiting ...\n");
		fflush(stdout);
		exit(1);
	}

	demo->connection = xcb_connect(NULL, &scr);
	if (xcb_connection_has_error(demo->connection) > 0) {
		printf("Cannot find a compatible Vulkan installable client driver (ICD).\nExiting ...\n");
		fflush(stdout);
		exit(1);
	}

	setup = xcb_get_setup(demo->connection);
	iter = xcb_setup_roots_iterator(setup);
	while (scr-- > 0) xcb_screen_next(&iter);

	demo->screen = iter.data;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	demo->display = wl_display_connect(NULL);

	if (demo->display == NULL) {
		printf("Cannot find a compatible Vulkan installable client driver (ICD).\nExiting ...\n");
		fflush(stdout);
		exit(1);
	}

	demo->registry = wl_display_get_registry(demo->display);
	wl_registry_add_listener(demo->registry, &registry_listener, demo);
	wl_display_dispatch(demo->display);
#endif
}

static void demo_init(struct demo *demo, int argc, char **argv) {
	memset(demo, 0, sizeof(*demo));

	#ifdef VK_USE_PLATFORM_WIN32_KHR
	demo->presentMode = VK_PRESENT_MODE_FIFO_KHR; //VK_PRESENT_MODE_FIFO_KHR //VK_PRESENT_MODE_IMMEDIATE_KHR
	#else
	demo->presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; //VK_PRESENT_MODE_FIFO_KHR //VK_PRESENT_MODE_IMMEDIATE_KHR
	#endif
	demo->frameCount = INT32_MAX;


	demo_init_connection(demo);

	demo_init_vk(demo);

	demo->width = 1280;
	demo->height = 720;

	demo->spin_angle = 0.0f;
	demo->spin_angle_val = 0.0f;
	demo->spin_increment = 0.0f;//0.2
	demo->pause = false;

	for (int i = 1; i < argc; i++) {

		if ((strcmp(argv[i], "--width") == 0) && (i < argc - 1)) {
			demo->width = atoi(argv[i + 1]);
			i++;
			continue;
		}
		if ((strcmp(argv[i], "--height") == 0) && (i < argc - 1)) {
			demo->height = atoi(argv[i + 1]);
			i++;
			continue;
		}
		if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1)) {
			demo->presentMode = atoi(argv[i + 1]);
			i++;
			continue;
		}
		fprintf(stderr,
			"Usage:\n  %s\t[--width <value>] [--height <value>] \n"
			"\t[--present_mode <present mode enum>]\n"
			"\t <present_mode_enum>\tVK_PRESENT_MODE_IMMEDIATE_KHR = %d\n"
			"\t\t\t\tVK_PRESENT_MODE_MAILBOX_KHR = %d\n"
			"\t\t\t\tVK_PRESENT_MODE_FIFO_KHR = %d\n"
			"\t\t\t\tVK_PRESENT_MODE_FIFO_RELAXED_KHR = %d\n",
			APP_SHORT_NAME, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR,
			VK_PRESENT_MODE_FIFO_RELAXED_KHR);
		fflush(stderr);
		exit(1);
	}
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
// Include header required for parsing the command line options.
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	MSG msg;    // message
	bool done;  // flag saying when app is complete
	int argc;
	char **argv;

	// Ensure wParam is initialized.
	msg.wParam = 0;

	// Use the CommandLine functions to get the command line arguments.
	// Unfortunately, Microsoft outputs
	// this information as wide characters for Unicode, and we simply want the
	// Ascii version to be compatible
	// with the non-Windows side.  So, we have to convert the information to
	// Ascii character strings.
	LPWSTR *commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (NULL == commandLineArgs) {
		argc = 0;
	}

	if (argc > 0) {
		argv = (char **)malloc(sizeof(char *) * argc);
		if (argv == NULL) {
			argc = 0;
		}
		else {
			for (int iii = 0; iii < argc; iii++) {
				size_t wideCharLen = wcslen(commandLineArgs[iii]);
				size_t numConverted = 0;

				argv[iii] = (char *)malloc(sizeof(char) * (wideCharLen + 1));
				if (argv[iii] != NULL) {
					wcstombs_s(&numConverted, argv[iii], wideCharLen + 1, commandLineArgs[iii], wideCharLen + 1);
				}
			}
		}
	}
	else {
		argv = NULL;
	}

	demo_init(&demo, argc, argv);

	// Free up the items we had to allocate for the command line arguments.
	if (argc > 0 && argv != NULL) {
		for (int iii = 0; iii < argc; iii++) {
			if (argv[iii] != NULL) {
				free(argv[iii]);
			}
		}
		free(argv);
	}

	demo.connection = hInstance;
	strncpy(demo.name, "VKme", APP_NAME_STR_LEN);
	demo_create_window(&demo);
	demo_init_vk_swapchain(&demo);

	demo_prepare(&demo);
	printf("Start \n");
	printf("Present mode: %s\n", demo.presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR ? "VK_PRESENT_MODE_IMMEDIATE_KHR" :
		demo.presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? "VK_PRESENT_MODE_MAILBOX_KHR" :
		demo.presentMode == VK_PRESENT_MODE_FIFO_KHR ? "VK_PRESENT_MODE_FIFO_KHR" :
		demo.presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR ? "VK_PRESENT_MODE_FIFO_RELAXED_KHR" : "nan");

	done = false;  // initialize loop condition variable

	ex_init();
				   // main message loop

	while (!done) {
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (msg.message == WM_QUIT)  // check for a quit message
		{
			done = true;  // if found, quit app
		}
		else {
			/* Translate and dispatch to event queue*/
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_KEYDOWN) {
			switch (msg.wParam)
			{
			case VK_SPACE:
				demo.pause = !demo.pause;
				break;
			case 0x31: // 1
				drawdebug = !drawdebug;
				break;
			default: break;
			}
		}

		if (msg.message == WM_MOUSEMOVE) {
			uniform_vals.u_mouse[0] = (int)LOWORD(msg.lParam);
			uniform_vals.u_mouse[1] = demo.height - (int)HIWORD(msg.lParam);
			switch (msg.wParam)
			{
			case WM_LBUTTONDOWN:if (!m_left_c)m_left = true; break;
			case WM_MBUTTONDOWN:break;
			case WM_RBUTTONDOWN:if (!m_right_c)m_right = true; break;
			}
			switch (msg.wParam)
			{
			case WM_LBUTTONUP:m_left = false; break;
			case WM_MBUTTONUP:break;
			case WM_RBUTTONUP:m_right = false; break;
			}
		}
		else {

			switch (msg.message)
			{
			case WM_LBUTTONDOWN:if (!m_left_c)m_left = true; break;
			case WM_MBUTTONDOWN:break;
			case WM_RBUTTONDOWN:if (!m_right_c)m_right = true; break;
			}
			switch (msg.message)
			{
			case WM_LBUTTONUP:m_left = false; break;
			case WM_MBUTTONUP:break;
			case WM_RBUTTONUP:m_right = false; break;
			}
		}
		RedrawWindow(demo.window, NULL, NULL, RDW_INTERNALPAINT);
	}

	clean_on_exit();
	demo_cleanup(&demo);

	return (int)msg.wParam;
}

#else
int main(int argc, char **argv) {
	struct demo demo;
	printf("Start \n");
	demo_init(&demo, argc, argv);
#if defined(VK_USE_PLATFORM_XCB_KHR)
	demo_create_xcb_window(&demo);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	demo_create_window(&demo);
#endif

	demo_init_vk_swapchain(&demo);
	demo_prepare(&demo);

	printf("Present mode: %s\n", demo.presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR ? "VK_PRESENT_MODE_IMMEDIATE_KHR" :
		demo.presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? "VK_PRESENT_MODE_MAILBOX_KHR" :
		demo.presentMode == VK_PRESENT_MODE_FIFO_KHR ? "VK_PRESENT_MODE_FIFO_KHR" :
		demo.presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR ? "VK_PRESENT_MODE_FIFO_RELAXED_KHR" : "nan");

	ex_init();

#if defined(VK_USE_PLATFORM_XCB_KHR)
	demo_run_xcb(&demo);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	demo_run(&demo);
#endif

	clean_on_exit();
	demo_cleanup(&demo);
	return validation_error;
}
#endif
