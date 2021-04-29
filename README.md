# vulkan shader launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else)

**Download** example build [**from Releases**](https://github.com/danilw/vulkan-shader-launcher/releases/tag/rel_1_1). Minimal binary build size, after upx compression, ~30kb. Using [yari-v](https://github.com/sheredom/yari-v) to compress shaders, in *example_fbo* size of bin(exe) with shaders ~50Kb.

**Dependencies** - only vulkan library(sdk), and win32/X11/Wayland for window and keyboard/mouse control.

**Source of code** - I made this code following cube-demo from VulkanSDK and [vktut](https://github.com/ShabbyX/vktut), this my code released under *The MIT License*.

**Compiler/platform support** - VisualStudio, GCC/Clang, Mingw. OS Windows and Linux(X11/Wayland)

**Wayland** - minimal setup with stable *xdg-shell*. Addef hotkey *f* as example of resize event.

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)
___
**Ultra minimal** *100 lines of code Vulkan launcher* link to [apollonian.c](https://github.com/przemyslawzaworski/CPP-Programming/blob/master/Vulkan/apollonian.c) or link to  [przemyslawzaworski](https://github.com/przemyslawzaworski/CPP-Programming) repository, przemyslawzaworski made very amazing small code!

___
**Example minimal** - single shader loader, shader from file. Control: Keyboard 1-debug, 2-vsynk 60fps, Space-pause. Binary build(exe) 26kb size.

**Example game** - single shader game example, drawing many elements in vulkan-drawcall loop, using Blend to draw mix color. Also using *yari-v* to compress shaders, and shaders build in to binary file. This example on video [youtube link](https://youtu.be/5Wzj-GNAo6c). Use *cmake* to build. 

**Example FBO** - [base on this shader](https://www.shadertoy.com/view/3syXDD) game that on *screenshot*, game logic on GPU, using framebuffer to store/read data, also *yari-v* and build in shaders. Use *cmake* to build. 

**Example images** - imgages/textures loading. [Used shader](https://www.shadertoy.com/view/lsfGWn) to test mipmaps, mipmaps supported. Default image format is *SRGB*, to change edit line 114 [example_images/main.c](https://github.com/danilw/vulkan-shader-launcher/blob/master/example_images/main.c#L114).

**use Validation layer** - use [VK_validation.sh](https://github.com/danilw/vulkan-shader-launcher/blob/master/use_validation_layer/VK_validation.sh) script for that. Requires VulkanSDK installed. This *shader launcher code* does not have validation errors.

**[Shadertoy launcher](https://github.com/danilw/vulkan-shadertoy-launcher)** - proper launcher for shadertoy shaders, 4 framebuffers, texture support. Moved to its own repository [vulkan-shadertoy-launcher](https://github.com/danilw/vulkan-shadertoy-launcher).

___
# Building

**Use cmake to build.** In Windows VS press *open Cmake*.
```
cd example_minimal
mkdir build
cd build
cmake ..
make
```
To build with Wayland in *CMakeLists.txt* set `OPTION(USE_WAYLAND_LINUX "use Wayland for Linux" ON)` by default used X11.


*Building without cmake:*

Build with **gcc** (linux): (to build with *clang* change *gcc* to *clang*)

X11:
```
cd example_minimal
gcc -DVK_USE_PLATFORM_XCB_KHR -O2 -s ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample -lm -lxcb -lvulkan
```
Wayland:
```
cd example_minimal
wayland-scanner client-header $wayland_protocols_dir/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
wayland-scanner private-code $wayland_protocols_dir/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.c

gcc -DVK_USE_PLATFORM_WAYLAND_KHR -O2 -s -I. -I/usr/include/wayland ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c xdg-shell-client-protocol.c main.c -o VKexample -lm -lxcb -lvulkan -lwayland-client
```

Build with **mingw64** (*vulkan-1.dll* from VulkanSDK, *vulkan.h* in system(cygwin or native) path):
```
x86_64-w64-mingw32-gcc -DVK_USE_PLATFORM_WIN32_KHR -O3 -s ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample.exe -lm -mwindows <path to>/vulkan-1.dll
```

**Images:** 

![img](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v2/scr1.jpg)

**Video:** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1yt.jpg)](https://youtu.be/5Wzj-GNAo6c)
