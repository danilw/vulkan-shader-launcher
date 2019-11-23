#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable
layout (location = 0) out vec4 uFragColor;

layout (location = 0) in vec3 frag_pos;
layout (location = 1) in float iTime;
layout (location = 2) in vec2 iResolution;
layout (location = 3) in vec2 draw_id;
layout (location = 4) in vec2 draw_pos_g;
layout (location = 5) in float FPS;
layout (location = 6) in vec2 iMouse;
layout (location = 7) in vec4 hpdata;
layout (location = 8) in vec2 u_valt;
layout (location = 9) in vec4 u_dataxx;


#include "main_c.glsl"


void main() {
   uFragColor=vec4(0.,0.,0.,1.);
   vec2 fragCoord=(frag_pos.xy/2.+vec2(0.5,0.5));
   fragCoord.y=1.-fragCoord.y;
   fragCoord*=iResolution;
   mainImage(uFragColor,fragCoord);
   
}
