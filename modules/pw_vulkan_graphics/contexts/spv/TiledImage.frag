#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 frontColour;
layout(location = 1) in vec2 pixelPos;

layout(push_constant) uniform PushConsts {
	vec4 screenBounds;
	vec2 imageLimits;
	float matrix[6];
} pc;

layout(binding = 0) uniform sampler2D imageTexture;

layout(location = 0) out vec4 outColour;

void main() { 
	mat2 transform = mat2 (pc.matrix[0], pc.matrix[3], pc.matrix[1], pc.matrix[4]);
	vec2 offset = vec2 (pc.matrix[2], pc.matrix[5]);

	vec2 texturePos = mod (transform * pixelPos + offset, pc.imageLimits);
	outColour = frontColour.a * texture (imageTexture, texturePos); 
}