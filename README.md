# vulkan_shader_launcher

**what is it** creating simple minimal "shader launcher" using Vulkan and C, for launch any shaders(from shadertoy or any else)

**v1** minimal launcher for my mini "shader game", everything in single shader using loop for draw. 

**Build**:
```
gcc -m64 -lm -lxcb -O3 -lvulkan -fdata-sections -ffunction-sections -Wl,--gc-sections vklauncher.c -o VKme
```

**binary build link** Win64 and Linux64

**Screenshot/Video** 

[![youtube](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/v1/v1.jpg)](https://youtu.be/5Wzj-GNAo6c)
