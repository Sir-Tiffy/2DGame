#version 150
precision highp float;
in vec4 out_colour;
in vec2 out_UV;
out vec4 fragColour;
uniform sampler2D tex;
void main(void){
	fragColour = texture2D(tex,out_UV)*out_colour;
}