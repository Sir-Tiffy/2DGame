#version 150
precision highp float;
in vec4 out_colour;
out vec4 fragColour;

void main(void){
	fragColour = out_colour;
}