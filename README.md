# vulkan shader launcher

Warning, this is old code, do not use it! use updated project code https://github.com/danilw/vulkan-shader-launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else)

Dependencies - only vulkan library(sdk), and win32/X11 for windows and keyboard/mouse control. Optional pthread, OpenAL.

Source of code - cube-demo from VulkanSDK, and [vktut](https://github.com/ShabbyX/vktut)

___

*v2*

use new updated code

___
*v1* finished, is single shader launcher

pack **bin** to have ~50kb bin size use `upx`

**v1_2** Updated *v1_1* with [yari-v](https://github.com/sheredom/yari-v) shader compression. Changes in *vklauncher.c* at `#define SPIRV_SHADER`.

To pack shader `*.spv to *.yariv` build *main.c* in *yariv* folder, using its `CMakeLists.txt` and move  result `yariv_pack` file at `yariv_to_hex.py` location, then launch *v1_2* building.
___

**v1_1** Updated *v1* with `CMakeLists.txt` and `spirv to hex` converter. 

**v1** minimal launcher for my mini "shader game", everything in single shader using loop for draw. 

Build:
```
gcc -s -m64 -lm -lxcb -O3 -lvulkan -fdata-sections -ffunction-sections -Wl,--gc-sections vklauncher.c -o VKme
```
Build with mingw64 (*vulkan-1.dll* from VulkanSDK):
```
x86_64-w64-mingw32-gcc -s -lm -O3 -lvulkan-1 -mwindows -fdata-sections -ffunction-sections -Wl,--gc-sections vklauncher.c -o VKme.exe
```

**v1 binary build link** Win64 and Linux64 [Download](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1.zip) (compressed build with *upx* and *yari-v*)

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)

**Images:** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1yt.jpg)](https://youtu.be/5Wzj-GNAo6c)
