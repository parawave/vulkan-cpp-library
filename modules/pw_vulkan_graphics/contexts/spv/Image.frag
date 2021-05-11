#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 frontColour;
layout(location = 1) in vec2 texturePos;

layout(binding = 0) uniform sampler2D imageTexture;

layout(location = 0) out vec4 outColour;

void main() { 
	outColour = frontColour.a * texture (imageTexture, texturePos); 
}