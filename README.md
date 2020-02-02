# vulkan shader launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else)

**Dependencies** - only vulkan library(sdk), and win32/X11 for window and keyboard/mouse control.

**Source of code** - cube-demo from VulkanSDK, and [vktut](https://github.com/ShabbyX/vktut)

**Compiler/platform support** - VisualStudio, GCC, Mingw. OS Windows and Linux(X11)

All binary builds in single zip. **Binary build download link** Windows **[Download Win64]()** and Linux **[Download Linux64]()**

___
**Example minimal** - single shader loader, shader from file. Control: Keyboard 1-debug, 2-vsynk 60fps, Space-pause.

**Example game** - single shader game example, drawing many elements in vulkan-drawcall loop. Also using *yari-v* to compress shaders, and shaders build in to binary file. \<TODO soon\>

**Example FBO** - [base on this shader](https://www.shadertoy.com/view/wdlGz8) single shader game(game logic on GPU) using framebuffer to store/read data, also *yari-v* and build in shaders. \<TODO soon\>

**Example shadertoy launcher** - proper launcher for shadertoy shaders, 4 framebuffers, texture support. \<TODO last\>

___
**Use cmake to build.**

*Building without cmake:*

Build (linux):
```

```
Build with mingw64 (*vulkan-1.dll* from VulkanSDK, use your system path):
```
x86_64-w64-mingw32-gcc -DVK_USE_PLATFORM_WIN32_KHR -O3 -s -lm -mwindows ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample.exe <path to>/vulkan-1.dll
```

*old code removed, everything udated, old code can be found* [link](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/vulkan-shader-launcher_old.zip)

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)

**Images:** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1yt.jpg)](https://youtu.be/5Wzj-GNAo6c)
