# vulkan shader launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else)

**Dependencies** - only vulkan library(sdk), and win32/X11 for window and keyboard/mouse control.

**Source of code** - I made this code following cube-demo from VulkanSDK and [vktut](https://github.com/ShabbyX/vktut), this my code released under *The MIT License*.

**Compiler/platform support** - VisualStudio, GCC/Clang, Mingw. OS Windows and Linux(X11)

All binary builds in single zip, **download [from releases](https://github.com/danilw/vulkan-shader-launcher/releases)**, or direct links Windows **[Download Win64](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v2/Vulkan_shader_launcher_examples.zip)** and Linux **[Download Linux64](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v2/Vulkan_shader_launcher_examples_linux.zip)**

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)
___
**Features** - framebuffers binding, resize support, minimize support, surfaces, blend, loading images, mipmaps, saving frames... and other look examples they display some of features.

___
**Ultra minimal** *100 lines of code Vulkan launcher* link to [apollonian.c](https://github.com/przemyslawzaworski/CPP-Programming/blob/master/Vulkan/apollonian.c) or link to  [przemyslawzaworski](https://github.com/przemyslawzaworski/CPP-Programming) repository, przemyslawzaworski made very amazing small code!

___
**Example minimal** - single shader loader, shader from file. Control: Keyboard 1-debug, 2-vsynk 60fps, Space-pause. Binary build(exe) 26kb size.

**Example game** - single shader game example, drawing many elements in vulkan-drawcall loop, using Blend to draw mix color. Also using *yari-v* to compress shaders, and shaders build in to binary file. This example on video [youtube link](https://youtu.be/5Wzj-GNAo6c).

**Example FBO** - [base on this shader](https://www.shadertoy.com/view/3syXDD) game that on *screenshot*, game logic on GPU, using framebuffer to store/read data, also *yari-v* and build in shaders.

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

*Building without cmake:*

Build with **gcc** (linux): (to build with *clang* change *gcc* to *clang*)
```
gcc -DVK_USE_PLATFORM_XCB_KHR -O2 -s ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample -lm -lxcb -lvulkan
```
Build with **mingw64** (*vulkan-1.dll* from VulkanSDK, *vulkan.h* in system(cygwin or native) path):
```
x86_64-w64-mingw32-gcc -DVK_USE_PLATFORM_WIN32_KHR -O3 -s ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample.exe -lm -mwindows <path to>/vulkan-1.dll
```

**Images:** 

![img](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v2/scr1.jpg)

**Video:** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1yt.jpg)](https://youtu.be/5Wzj-GNAo6c)
