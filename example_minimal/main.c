// Danil, 2020 Vulkan shader launcher, self https://github.com/danilw/vulkan-shader-launcher
// code based on Shabi's Vulkan Tutorials https://github.com/ShabbyX/vktut

#include <stdio.h>
#include <stdlib.h>

#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <unistd.h>
#endif

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#endif

#include <time.h>
#include "../vk_utils/vk_utils.h"
#include "../vk_utils/vk_render_helper.h"
#include "../os_utils/utils.h"

#include "debug.h"

struct shaders_uniforms{ //same to shadertoy
	float iMouse[4];//iMouse.xy updated always
	float iDate[4];
	int iMouse_lr[2]; //is mouse left[0], right[1] clicked
	float iResolution[2];
	int debugdraw;
	int pause;
	float iTime;
	float iTimeDelta;
	int iFrame;
};

enum
{
	BUFFER_VERTICES = 0,
	BUFFER_INDICES = 1,
};
enum
{
	SHADER_MAIN_VERTEX = 0,
	SHADER_MAIN_FRAGMENT = 1,
};

struct render_data
{
	struct objects
	{
		struct vertex
		{
			float pos[3];
		} vertices[4];

		uint16_t indices[4];
	} objects;

	struct shaders_uniforms push_constants;

	struct vk_buffer buffers[2];
	struct vk_shader shaders[2];
	struct vk_graphics_buffers *main_gbuffers;

	VkRenderPass main_render_pass;
	struct vk_layout main_layout;
	struct vk_pipeline main_pipeline;
	VkDescriptorSet main_desc_set;

};

struct render_data render_data = { .main_gbuffers = NULL, };
VkInstance vk;
struct vk_physical_device phy_dev;
struct vk_device dev;
struct vk_swapchain swapchain = {0};
struct app_os_window os_window;
struct vk_render_essentials essentials;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include <shellapi.h>

static bool render_loop_draw(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window);
static bool on_window_resize(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials, 
	struct vk_swapchain *swapchain, struct render_data *render_data, struct app_os_window *os_window);

#include "../os_utils/os_win_utils.h"
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include "../os_utils/xcb_x11_utils.h"
#endif

static vk_error allocate_render_data(struct vk_physical_device *phy_dev, struct vk_device *dev,
		struct vk_swapchain *swapchain, struct vk_render_essentials *essentials, struct render_data *render_data, bool reload_shaders)
{
	
	static bool load_once=false;
	vk_error retval = VK_ERROR_NONE;
	VkResult res;
	

	render_data->buffers[BUFFER_VERTICES] = (struct vk_buffer){
		.size = sizeof render_data->objects.vertices,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.host_visible = false,
	};

	render_data->buffers[BUFFER_INDICES] = (struct vk_buffer){
		.size = sizeof render_data->objects.indices,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.host_visible = false,
	};

	retval = vk_create_buffers(phy_dev, dev, render_data->buffers, 2);
	if (!vk_error_is_success(&retval))
	{
		vk_error_printf(&retval, "Failed to create vertex, index and transformation buffers\n");
		return retval;
	}
	
	if(!load_once){
		render_data->objects = (struct objects){
			.vertices = {
				[0] = (struct vertex){ .pos = { 1.0,  1.0, 0.0}, },
				[1] = (struct vertex){ .pos = { 1.0, -1.0, 0.0}, },
				[2] = (struct vertex){ .pos = {-1.0,  1.0, 0.0}, },
				[3] = (struct vertex){ .pos = {-1.0, -1.0, 0.0}, },
			},
			.indices = {
				0, 1, 2, 3,
			},
		};
	}

	retval = vk_render_init_buffer(phy_dev, dev, essentials, &render_data->buffers[BUFFER_VERTICES], render_data->objects.vertices, "vertex");
	if (!vk_error_is_success(&retval))
		return retval;
	retval = vk_render_init_buffer(phy_dev, dev, essentials, &render_data->buffers[BUFFER_INDICES], render_data->objects.indices, "index");
	if (!vk_error_is_success(&retval))
		return retval;

	if((!load_once)||(reload_shaders)){
		render_data->shaders[SHADER_MAIN_VERTEX] = (struct vk_shader){
			.spirv_file = "shaders/spv/main.vert.spv",
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
		};
		render_data->shaders[SHADER_MAIN_FRAGMENT] = (struct vk_shader){
			.spirv_file = "shaders/spv/main.frag.spv",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		retval = vk_load_shaders(dev, render_data->shaders, 2);
		if (!vk_error_is_success(&retval))
		{
			vk_error_printf(&retval, "Could not load the shaders\n");
			return retval;
		}
	}
	render_data->main_gbuffers = malloc(essentials->image_count * sizeof *render_data->main_gbuffers);
	for (uint32_t i = 0; i < essentials->image_count; ++i)
		render_data->main_gbuffers[i] = (struct vk_graphics_buffers){
			.surface_size = swapchain->surface_caps.currentExtent,
			.swapchain_image = essentials->images[i],
		};

	// 8bit BGRA for main_image VK_FORMAT_B8G8R8A8_UNORM
	retval = vk_create_graphics_buffers(phy_dev, dev, swapchain->surface_format.format, render_data->main_gbuffers, essentials->image_count,
			&render_data->main_render_pass, VK_C_CLEAR, VK_WITHOUT_DEPTH);
	if (!vk_error_is_success(&retval))
	{
		vk_error_printf(&retval, "Could not create graphics buffers\n");
		return retval;
	}
	
	/*******************
	 * MAIN_IMAGE PART *
	 *******************/

	/* Layouts */
	
	VkPushConstantRange push_constant_range = {
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof render_data->push_constants,
	};
	
	struct vk_resources resources = {
		.buffers = render_data->buffers,
		.buffer_count = 2,
		.shaders = render_data->shaders,
		.shader_count = 2,
		.push_constants = &push_constant_range,
		.push_constant_count = 1,
		.render_pass = render_data->main_render_pass,
	};
	render_data->main_layout = (struct vk_layout){
		.resources = &resources,
	};
	retval = vk_make_graphics_layouts(dev, &render_data->main_layout, 1);
	if (!vk_error_is_success(&retval))
	{
		vk_error_printf(&retval, "Could not create descriptor set or pipeline layouts\n");
		return retval;
	}
	
	/* Pipeline */
	VkVertexInputBindingDescription vertex_binding = {
		.binding = 0,
		.stride = sizeof *render_data->objects.vertices,
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	VkVertexInputAttributeDescription vertex_attributes[1] = {
		[0] = {
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0,
		},
	};
	render_data->main_pipeline = (struct vk_pipeline){
		.layout = &render_data->main_layout,
		.vertex_input_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &vertex_binding,
			.vertexAttributeDescriptionCount = 1,
			.pVertexAttributeDescriptions = vertex_attributes,
		},
		.input_assembly_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		},
		.tessellation_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		},
		.thread_count = 1,
	};

	retval = vk_make_graphics_pipelines(dev, &render_data->main_pipeline, 1, false);
	if (!vk_error_is_success(&retval))
	{
		vk_error_printf(&retval, "Could not create graphics pipeline\n");
		return retval;
	}
	
	/* Descriptor Set */
	VkDescriptorSetAllocateInfo set_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = render_data->main_pipeline.set_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &render_data->main_layout.set_layout,
	};
	res = vkAllocateDescriptorSets(dev->device, &set_info, &render_data->main_desc_set);
	retval = VK_ERROR_NONE;
	vk_error_set_vkresult(&retval, res);
	if (res)
	{
		vk_error_printf(&retval, "Could not allocate descriptor set from pool\n");
		return retval;
	}

	load_once=true;
	
	return retval;
}

static void free_render_data(struct vk_device *dev, struct vk_render_essentials *essentials, struct render_data *render_data)
{
	vkDeviceWaitIdle(dev->device);

	vk_free_pipelines(dev, &render_data->main_pipeline, 1);
	vk_free_layouts(dev, &render_data->main_layout, 1);
	vk_free_buffers(dev, render_data->buffers, 2);
	vk_free_shaders(dev, render_data->shaders, 2);
	vk_free_graphics_buffers(dev, render_data->main_gbuffers, essentials->image_count, render_data->main_render_pass);

	free(render_data->main_gbuffers);
}

static void render_loop_init(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
	int res;
	vk_error retval = VK_ERROR_NONE;

	
	res = vk_render_get_essentials(&essentials, phy_dev, dev, swapchain);
	if (res){
		vk_render_cleanup_essentials(&essentials, dev);
		return;
	}

	retval = allocate_render_data(phy_dev, dev, swapchain, &essentials, &render_data, os_window->reload_shaders_on_resize);
	if (!vk_error_is_success(&retval))
	{
		free_render_data(dev, &essentials, &render_data);
		vk_render_cleanup_essentials(&essentials, dev);
		return;
	}
	
	os_window->prepared = true;
	os_window->resize_event=false;
	
    return;
    
}

static void exit_cleanup_render_loop(struct vk_device *dev, struct vk_render_essentials *essentials, struct render_data *render_data)
{
	vkDeviceWaitIdle(dev->device);
	free_render_data(dev, essentials, render_data);
	vk_render_cleanup_essentials(essentials, dev);
}

static void exit_cleanup(VkInstance vk, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
    vk_free_swapchain(vk, dev, swapchain);
    vk_cleanup(dev);
#if defined(VK_USE_PLATFORM_XCB_KHR)
	xcb_destroy_window(os_window->connection, os_window->xcb_window);
	xcb_disconnect(os_window->connection);
	free(os_window->atom_wm_delete_window);
#endif
    vk_exit(vk);
}


static bool on_window_resize(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials, 
	struct vk_swapchain *swapchain, struct render_data *render_data, struct app_os_window *os_window)
{
	vk_error res = VK_ERROR_NONE;
	
	if(!os_window->prepared )return true;
	
	vkDeviceWaitIdle(dev->device);
	os_window->prepared = false;
	
	vk_free_pipelines(dev, &render_data->main_pipeline, 1);
	vk_free_graphics_buffers(dev, render_data->main_gbuffers, essentials->image_count, render_data->main_render_pass);
	vk_free_layouts(dev, &render_data->main_layout, 1);
	vk_free_buffers(dev, render_data->buffers, 2);
	
	if(os_window->reload_shaders_on_resize)
		vk_free_shaders(dev, render_data->shaders, 2);
	
	vk_render_cleanup_essentials(essentials, dev);

	free(render_data->main_gbuffers);
	
	res = vk_get_swapchain(vk, phy_dev, dev, swapchain, os_window, 1, &os_window->present_mode);
	if (vk_error_is_error(&res))
	{
		vk_error_printf(&res, "Could not create surface and swapchain\n");
		exit_cleanup(vk, dev, swapchain, os_window);
        return false;
	}
	
	render_loop_init(phy_dev, dev, swapchain, os_window);
	
	return true;
}

void update_params(struct app_data_struct *app_data, bool fps_lock){
	float rdelta=0;
	if(fps_lock)FPS_LOCK(60);
	float delta=update_fps_delta();
	if(!app_data->pause){
		app_data->iFrame++;
		app_data->iTime+=delta;
	}
	app_data->iTimeDelta=delta;
	float pause_time=pres_pause(app_data->pause);
}

static bool render_loop_draw(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
    int res;
	vk_error retval = VK_ERROR_NONE;

	uint32_t image_index;

	res = vk_render_start(&essentials, dev, swapchain, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &image_index);
	if (res)
		return false;

	VkClearValue clear_values = {
		.color = { .float32 = {0.0, 0.0, 0.0, 1.0}, },
	};
	VkRenderPassBeginInfo pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_data.main_render_pass,
		.framebuffer = render_data.main_gbuffers[image_index].framebuffer,
		.renderArea = {
			.offset = { .x = 0, .y = 0, },
			.extent = render_data.main_gbuffers[image_index].surface_size,
		},
		.clearValueCount = 1,
		.pClearValues = &clear_values,
	};

	vkCmdBeginRenderPass(essentials.cmd_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(essentials.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data.main_pipeline.pipeline);

	vkCmdBindDescriptorSets(essentials.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			render_data.main_layout.pipeline_layout, 0, 1, &render_data.main_desc_set, 0, NULL);

	VkDeviceSize vertices_offset = 0;
	vkCmdBindVertexBuffers(essentials.cmd_buffer, 0, 1, &render_data.buffers[BUFFER_VERTICES].buffer, &vertices_offset);
    vkCmdBindIndexBuffer(essentials.cmd_buffer, render_data.buffers[BUFFER_INDICES].buffer, 0, VK_INDEX_TYPE_UINT16);

	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = os_window->app_data.iResolution[0],
		.height = os_window->app_data.iResolution[1],
		.minDepth = 0,
		.maxDepth = 1,
	};
	vkCmdSetViewport(essentials.cmd_buffer, 0, 1, &viewport);

	VkRect2D scissor = {
		.offset = { .x = 0, .y = 0, },
		.extent = render_data.main_gbuffers[image_index].surface_size,
	};
	vkCmdSetScissor(essentials.cmd_buffer, 0, 1, &scissor);
	
	
	struct my_time_struct my_time;
	get_local_time(&my_time);
	float day_sec=((float)my_time.msec)/1000.0+my_time.sec+my_time.min*60+my_time.hour*3600;
	
	
	render_data.push_constants = (struct shaders_uniforms){
        .iResolution[0]=os_window->app_data.iResolution[0],
        .iResolution[1]=os_window->app_data.iResolution[1],
        .iTime=os_window->app_data.iTime,
        .iTimeDelta=os_window->app_data.iTimeDelta,
        .iFrame=os_window->app_data.iFrame,
        .iMouse[0]=os_window->app_data.iMouse[0],
        .iMouse[1]=os_window->app_data.iMouse[1],
        .iMouse[2]=os_window->app_data.iMouse_lclick[0],
        .iMouse[3]=os_window->app_data.iMouse_lclick[1],
        .iMouse_lr[0]=(int)os_window->app_data.iMouse_click[0],
        .iMouse_lr[1]=(int)os_window->app_data.iMouse_click[1],
        .iDate[0]=my_time.year,
        .iDate[1]=my_time.month,
        .iDate[2]=my_time.day,
        .iDate[3]=day_sec,
        .debugdraw=(int)os_window->app_data.drawdebug,
        .pause=(int)os_window->app_data.pause,
    };

    vkCmdPushConstants(essentials.cmd_buffer, render_data.main_layout.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
            sizeof render_data.push_constants, &render_data.push_constants);

	vkCmdDraw(essentials.cmd_buffer, 4, 1, 0, 0);
	//vkCmdDrawIndexed(essentials.cmd_buffer, 4, 1, 0, 0, 0);

	vkCmdEndRenderPass(essentials.cmd_buffer);

	res = vk_render_finish(&essentials, dev, swapchain, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, image_index, NULL, NULL);
	
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        os_window->resize_event=true;
        res = 0;
    } else if (res == VK_ERROR_SURFACE_LOST_KHR) {
		vkDestroySurfaceKHR(vk, swapchain->surface, NULL);
		VkResult tres;
		tres=vk_create_surface(vk, swapchain, os_window);
		vk_error_set_vkresult(&retval, tres);
		if (tres)
			return false;
        os_window->resize_event=true;
        res = 0;
    }
	
	if (res)
		return false;
	
	update_params(&os_window->app_data,os_window->fps_lock);
	return true;
	
}

void init_win_params(struct app_os_window *os_window){
    os_window->app_data.iResolution[0]=1280;
    os_window->app_data.iResolution[1]=720;
    os_window->app_data.iFrame=0;
    os_window->app_data.iMouse[0]=0;
    os_window->app_data.iMouse[1]=0;
    os_window->app_data.iMouse_click[0]=false;
    os_window->app_data.iMouse_click[1]=false;
    os_window->app_data.iMouse_lclick[0]=0;
    os_window->app_data.iMouse_lclick[1]=0;
    os_window->app_data.iTime=0;
    os_window->app_data.pause=false;
    os_window->app_data.quit=false;
    os_window->app_data.drawdebug=false;
    os_window->fps_lock=false;
    os_window->is_minimized=false;
    os_window->prepared=false;
    os_window->resize_event=false;
    os_window->reload_shaders_on_resize=false;
    os_window->print_debug=false;
    strncpy(os_window->name, "Vulkan Shader launcher | twitter.com/AruGL", APP_NAME_STR_LEN);
}

#if defined(VK_USE_PLATFORM_XCB_KHR)
static void render_loop_xcb(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
	while (!os_window->app_data.quit)
	{
        xcb_generic_event_t *event;

        if (os_window->app_data.pause) {
            event = xcb_wait_for_event(os_window->connection);
        }
        else {
            event = xcb_poll_for_event(os_window->connection);
        }
        while (event) {
            app_handle_xcb_event(os_window, event);
            free(event);
            event = xcb_poll_for_event(os_window->connection);
        }
        if((!os_window->is_minimized)&&(!os_window->resize_event)) {
			if(!os_window->app_data.quit){
				os_window->app_data.quit=!render_loop_draw(phy_dev, dev, swapchain, os_window);
			}
            else break;
        }
        else {
			if((!os_window->is_minimized)&&os_window->resize_event){
				on_window_resize(phy_dev, dev, &essentials, swapchain, &render_data, os_window); //execute draw or resize per frame, not together
			}
		}
        if(os_window->is_minimized){ //I do not delete everything on minimize, only stop rendering
			sleep_ms(10);
		}
    }
    exit_cleanup_render_loop(dev, &essentials, &render_data);
}
#endif

void print_usage(char *name){
	printf("Usage: %s \n"
			"\t[--present_mode <present mode enum>]\n"
			"\t <present_mode_enum>\tVK_PRESENT_MODE_IMMEDIATE_KHR = %d\n"
			"\t\t\t\tVK_PRESENT_MODE_MAILBOX_KHR = %d\n"
			"\t\t\t\tVK_PRESENT_MODE_FIFO_KHR = %d\n"
			"\t\t\t\tVK_PRESENT_MODE_FIFO_RELAXED_KHR = %d\n"
			"\t[--debug]\n"
			"\t[--reload_shaders] will reload shaders form file on resize\n"
			"Control: Keyboard 1-debug, 2-vsynk 60fps, Space-pause\n", name, 
			VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, 
			VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	MSG msg;
	bool done;
	int argc;
	char **argv;

	msg.wParam = 0;

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
    
    vk_error res =VK_ERROR_NONE;
	int retval = EXIT_FAILURE;
    
    init_win_params(&os_window);
	uint32_t dev_count = 1;
	os_window.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	
	if(argc>1){
		if (strcmp(argv[1], "--help") == 0)
		{
			SetStdOutToNewConsole();
			print_usage(argv[0]);
			Sleep(44000);
			return 0;
		}
	}
	
	for (int i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1)) {
			os_window.present_mode = atoi(argv[i + 1]);
			i++;
			continue;
		}
		if (strcmp(argv[i], "--debug") == 0)
		{
			os_window.print_debug=true;
			continue;
		}
		if (strcmp(argv[i], "--reload_shaders") == 0)
		{
			os_window.reload_shaders_on_resize=true;
			continue;
		}
	}

    

	if (argc > 0 && argv != NULL) {
		for (int iii = 0; iii < argc; iii++) {
			if (argv[iii] != NULL) {
				free(argv[iii]);
			}
		}
		free(argv);
	}
	
	if(os_window.print_debug){
		SetStdOutToNewConsole();
	}

	os_window.connection = hInstance;
    
    res = vk_init(&vk);
	if (!vk_error_is_success(&res))
	{
		vk_error_printf(&res, "Could not initialize Vulkan\n");
		return retval;
	}

	res = vk_enumerate_devices(vk, &phy_dev, &dev_count);
	if (vk_error_is_error(&res))
	{
		vk_error_printf(&res, "Could not enumerate devices\n");
		vk_exit(vk);
        return retval;
	}

	if (dev_count < 1)
	{
		printf("No graphics card? Shame on you\n");
		vk_exit(vk);
        return retval;
	}

	res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT);
	if (vk_error_is_error(&res))
	{
		vk_error_printf(&res, "Could not setup logical device, command pools and queues\n");
        vk_cleanup(&dev);
		vk_exit(vk);
        return retval;
	}

    app_create_window(&os_window);
    
    swapchain.swapchain = VK_NULL_HANDLE;
	res = vk_get_swapchain(vk, &phy_dev, &dev, &swapchain, &os_window, 1, &os_window.present_mode);
	if (vk_error_is_error(&res))
	{
		vk_error_printf(&res, "Could not create surface and swapchain\n");
		exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
	}
	
	if(os_window.print_debug){
		struct vk_physical_device **dptrs;
		dptrs = malloc(dev_count * sizeof(struct vk_physical_device*));
		dptrs[0]=&phy_dev;
		struct vk_swapchain **sptrs;
		sptrs = malloc(dev_count * sizeof(struct vk_swapchain*));
		sptrs[0]=&swapchain;
		print_vkinfo(dptrs, sptrs, dev_count, os_window.present_mode);
	}
    
    render_loop_init(&phy_dev, &dev, &swapchain, &os_window);
	done = false;
	while (!done) {
		PeekMessage(&msg,0,0,0,PM_REMOVE);
		process_msg(&msg,&done);
		RedrawWindow(os_window.window, NULL, NULL, RDW_INTERNALPAINT);
	}
    
    exit_cleanup_render_loop(&dev, &essentials, &render_data);
    exit_cleanup(vk, &dev, &swapchain, &os_window);
    
	return (int)msg.wParam;
}

#elif defined(VK_USE_PLATFORM_XCB_KHR)

int main(int argc, char **argv)
{
	vk_error res;
	int retval = EXIT_FAILURE;
	VkInstance vk;
	struct vk_physical_device phy_dev;
	struct vk_device dev;
	struct vk_swapchain swapchain = {0};
	struct app_os_window os_window;
    init_win_params(&os_window);
	uint32_t dev_count = 1;
	os_window.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	
	if(argc>1){
		if (strcmp(argv[1], "--help") == 0)
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	
	for (int i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1)) {
			os_window.present_mode = atoi(argv[i + 1]);
			i++;
			continue;
		}
		if (strcmp(argv[i], "--debug") == 0)
		{
			os_window.print_debug=true;
			continue;
		}
		if (strcmp(argv[i], "--reload_shaders") == 0)
		{
			os_window.reload_shaders_on_resize=true;
			continue;
		}
	}

	srand(time(NULL));

	res = vk_init(&vk);
	if (!vk_error_is_success(&res))
	{
		vk_error_printf(&res, "Could not initialize Vulkan\n");
		return retval;
	}

	res = vk_enumerate_devices(vk, &phy_dev, &dev_count);
	if (vk_error_is_error(&res))
	{
		vk_error_printf(&res, "Could not enumerate devices\n");
		vk_exit(vk);
        return retval;
	}

	if (dev_count < 1)
	{
		printf("No graphics card? Shame on you\n");
		vk_exit(vk);
        return retval;
	}

	res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT);
	if (vk_error_is_error(&res))
	{
		vk_error_printf(&res, "Could not setup logical device, command pools and queues\n");
        vk_cleanup(&dev);
		vk_exit(vk);
        return retval;
	}

    printf("Init XCB\n");
    app_init_connection(&os_window);
	app_create_xcb_window(&os_window);
	

	swapchain.swapchain = VK_NULL_HANDLE;
	res = vk_get_swapchain(vk, &phy_dev, &dev, &swapchain, &os_window, 1, &os_window.present_mode);
	if (vk_error_is_error(&res))
	{
		vk_error_printf(&res, "Could not create surface and swapchain\n");
		exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
	}
    
	if(os_window.print_debug){
		struct vk_physical_device **dptrs;
		dptrs = malloc(dev_count * sizeof(struct vk_physical_device*));
		dptrs[0]=&phy_dev;
		struct vk_swapchain **sptrs;
		sptrs = malloc(dev_count * sizeof(struct vk_swapchain*));
		sptrs[0]=&swapchain;
		print_vkinfo(dptrs, sptrs, dev_count, os_window.present_mode);
	}

	render_loop_init(&phy_dev, &dev, &swapchain, &os_window);
	render_loop_xcb(&phy_dev, &dev, &swapchain, &os_window);

	retval = 0;

    exit_cleanup(vk, &dev, &swapchain, &os_window);
    return retval;
}

#endif
