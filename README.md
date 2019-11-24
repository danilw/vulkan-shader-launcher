# vulkan_shader_launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else)
___

**v2** *TODO* fbo support, better code...soon...
___

pack **bin** for *v1_2* after build original size of *v1_2* is ~150kb to make it ~51kb use `strip VKme` and `upx VKme`

**v1_2** Updated *v1_1* with [yari-v](https://github.com/sheredom/yari-v) shader compression. Changes in *vklauncher.c* at `#define SPIRV_SHADER`.

To pack shader `*.spv to *.yariv` build *main.c* in *yariv* folder, using its `CMakeLists.txt` and move  result `yariv_pack` file at `yariv_to_hex.py` location, then launch *v1_2* building.
___

pack **bin**, after build original size of *v1* is ~250kb to make it ~78kb use `strip VKme` and `upx VKme`

**v1_1** Updated *v1* with `CMakeLists.txt` and `spirv to hex` converter. 

**v1** minimal launcher for my mini "shader game", everything in single shader using loop for draw. Build:
```
gcc -m64 -lm -lxcb -O3 -lvulkan -fdata-sections -ffunction-sections -Wl,--gc-sections vklauncher.c -o VKme
```

**v1 binary build link** Win64 and Linux64 [Download](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1.zip) (compressed build with *upx* and *yari-v*)

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)

**Images:** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1yt.jpg)](https://youtu.be/5Wzj-GNAo6c)
