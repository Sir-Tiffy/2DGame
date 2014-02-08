#version 150
in vec2 in_Position;
in vec4 in_Colour;
out vec4 out_colour;
uniform mat4 vp;

void main(void){
	out_colour = in_Colour;
	gl_Position = vp*vec4(in_Position,0,1);
}