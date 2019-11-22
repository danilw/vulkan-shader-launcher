# vulkan_shader_launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else)

**v2** *TODO* fbo support, better code...soon...
___

pack **bin**, original size of *v1* is ~250kb to make it ~78kb use  `strip VKme` and `upx VKme`

**v1_1** Updated *v1* with `CMakeLists.txt` and `spirv to hex` converter. 

**v1** minimal launcher for my mini "shader game", everything in single shader using loop for draw. Build:
```
gcc -m64 -lm -lxcb -O3 -lvulkan -fdata-sections -ffunction-sections -Wl,--gc-sections vklauncher.c -o VKme
```

**binary build link** Win64 and Linux64  [Download](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1.zip)

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)

**Screenshot/Video** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1yt.jpg)](https://youtu.be/5Wzj-GNAo6c)

![scr](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1.jpg)
