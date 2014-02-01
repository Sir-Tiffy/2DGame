#version 150
in vec2 in_Position;
in vec3 in_Colour;
in vec2 in_UV;
out vec3 out_colour;
out vec2 out_UV;
uniform mat4 vp;

#define width 1200
#define height 640

void main(void){
	out_colour = in_Colour;
	out_UV = in_UV;
	gl_Position = vp*vec4(in_Position,0,1);
	//gl_Position=(floor((vp*vec4(in_Position,0,1))*vec4(width,height,1,1)))/vec4(width,height,1,1);
}