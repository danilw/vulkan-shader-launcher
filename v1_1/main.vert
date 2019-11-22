#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout(std140, binding = 0) uniform buf {
        vec4 position[2*3];
        vec2 u_time; //[time,fps]
        vec2 u_resolution;
        vec2 draw_id;
        vec2 draw_pos;
        vec2 u_mouse;
        vec2 u_valt;
        vec4 u_hpdata;
        vec4 u_dataxx;
} ubuf;

layout (location = 0) out vec3 frag_pos;
layout (location = 1) out float iTime;
layout (location = 2) out vec2 iResolution;
layout (location = 3) out vec2 draw_id;
layout (location = 4) out vec2 draw_pos_g;
layout (location = 5) out float FPS;
layout (location = 6) out vec2 iMouse;
layout (location = 7) out vec4 hpdata;
layout (location = 8) out vec2 u_valt;
layout (location = 9) out vec4 u_dataxx;

void main() 
{
   gl_Position = ubuf.position[gl_VertexIndex];
   iTime=ubuf.u_time.x;
   FPS=ubuf.u_time.y;
   iResolution=ubuf.u_resolution;
   draw_id=ubuf.draw_id;
   draw_pos_g=ubuf.draw_pos;
   iMouse=ubuf.u_mouse;
   hpdata=ubuf.u_hpdata;
   u_valt=ubuf.u_valt;
   u_dataxx=ubuf.u_dataxx;
   frag_pos = gl_Position.xyz;
}
