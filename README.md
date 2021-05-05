# vulkan shader launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else). Without validation errors and crossplatform.

**Download** example build [**from Releases**](https://github.com/danilw/vulkan-shader-launcher/releases/tag/rel_1_1). Minimal binary build size, after upx compression, ~30kb. Using [yari-v](https://github.com/sheredom/yari-v) to compress shaders, in *example_fbo* size of bin(exe) with shaders ~50Kb.

**Dependencies** - only vulkan library(sdk), and win32/X11/Wayland for window and keyboard/mouse control.

**Source of code** - I made this code following cube-demo from VulkanSDK and [vktut](https://github.com/ShabbyX/vktut), this my code released under *The MIT License*.

**Compiler/platform support** - VisualStudio, GCC/Clang, Mingw. OS Windows and Linux(X11/Wayland)

**Wayland** - minimal setup with stable *xdg-shell*. Added hotkey *f* as example of resize event.

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)
___
**Ultra minimal** *100 lines of code Vulkan launcher* link to [apollonian.c](https://github.com/przemyslawzaworski/CPP-Programming/blob/master/Vulkan/apollonian.c) or link to  [przemyslawzaworski](https://github.com/przemyslawzaworski/CPP-Programming) repository, przemyslawzaworski made very amazing small code!

___
**Example minimal** - single shader loader, shader from file. Control: Keyboard 1-debug, 2-vsynk 60fps, Space-pause. Binary build(exe) 30kb size.

**Example game** - single shader game example, drawing many elements in vulkan-drawcall loop, using Blend to draw mix color. Also using *yari-v* to compress shaders, and shaders build in to binary file. This example on video [youtube link](https://youtu.be/5Wzj-GNAo6c).

**Example FBO** - [base on this shader](https://www.shadertoy.com/view/3syXDD) game that on *screenshot*, game logic on GPU, using framebuffer to store/read data, also *yari-v* and build in shaders.

**Example images** - imgages/textures loading. [Used shader](https://www.shadertoy.com/view/lsfGWn) to test mipmaps, mipmaps supported. Default image format is *SRGB*, to change edit line 114 [example_images/main.c](https://github.com/danilw/vulkan-shader-launcher/blob/master/example_images/main.c#L114).

**use Validation layer** - use [VK_validation.sh](https://github.com/danilw/vulkan-shader-launcher/blob/master/use_validation_layer/VK_validation.sh) script for that. Requires VulkanSDK installed. This *shader launcher code* does not have validation errors.

**[Shadertoy launcher](https://github.com/danilw/vulkan-shadertoy-launcher)** - proper launcher for shadertoy shaders, 4 framebuffers, texture support. Moved to its own repository [vulkan-shadertoy-launcher](https://github.com/danilw/vulkan-shadertoy-launcher).

___
# Building

**Use cmake to build.**

Using VS2019 in Windows (open links to see tutorial screenshot):

0. Download and install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows).
1. Launch VS press [Continue without code](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/1.png) and [File-Open-CMake](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/2.png) select [CMakeLists.txt file](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/3.png) from one of the *example_* folders.
2. Press [Manage Configureations](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/4.png) and [Add new configuration](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/5.png) select [x64 Release](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/6.png).
3. In General [Configuration type set MinSizeRel](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/7.png) and press Ctrl+S to save.
4. Then [select created configuration](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/8.png) and press [Build-Build All](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/9.png).
5. Compiled build location *example_<selected>\out\build\x64-Release\<name>.exe*.


In Linux use command:
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
gcc -DVK_USE_PLATFORM_XCB_KHR -O2 -s ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample -lm -lxcb -lvulkan
```
Wayland:
```
cp ../wayland_xdg/xdg-shell-client-protocol.c xdg-shell-client-protocol.c
cp ../wayland_xdg/xdg-shell-client-protocol.h xdg-shell-client-protocol.h
gcc -DVK_USE_PLATFORM_WAYLAND_KHR -O2 -s -I. -I/usr/include/wayland ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c xdg-shell-client-protocol.c main.c -o VKexample -lm -lvulkan -lwayland-client
```

to generare Wayland xdg-shell headers (from wayland_xdg folder):
```
wayland_protocols_dir=$(pkg-config --variable=pkgdatadir wayland-protocols)
wayland-scanner client-header $wayland_protocols_dir/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
wayland-scanner private-code $wayland_protocols_dir/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.c
```

Build with **mingw64** (*vulkan-1.dll* from VulkanSDK, *vulkan.h* in system(cygwin or native) path):
```
x86_64-w64-mingw32-gcc -DVK_USE_PLATFORM_WIN32_KHR -O3 -s ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample.exe -lm -mwindows <path to>/vulkan-1.dll
```
(in the *example game and fbo* add `-DYARIV_SHADER` to build command to have yari-v shaders in the bin(exe), to compile yari-v shader use *shaders/build_shaders_yariv.sh*)

**Images:** 

![img](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v2/scr1.jpg)

**Video:** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1yt.jpg)](https://youtu.be/5Wzj-GNAo6c)
