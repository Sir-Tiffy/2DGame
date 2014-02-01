#pragma once

const static char* VERT =
"#version 150\n"
"in vec2 in_Position;\n"
"in vec3 in_Colour;\n\
in vec2 in_UV;\n\
out vec3 out_colour;\n\
out vec2 out_UV;\n\
uniform mat4 vp;\n\
void main(void){;\n\
out_colour = in_Colour;\n\
out_UV = in_UV;\n\
gl_Position=vp*vec4(in_Position,0,1);\n\
}";

const static char* FRAG = 
"#version 150\n\
precision highp float;\n\
in vec3 out_colour;\n\
in vec2 out_UV;\n\
out vec4 fragColour;\
uniform sampler2D tex;\n\
void main(void){\n\
fragColour = texture2D(tex,out_UV)*vec4(out_colour,1);\n\
}";