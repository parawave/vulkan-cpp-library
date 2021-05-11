#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 frontColour;
layout(location = 1) in vec2 pixelPos;

layout(push_constant) uniform PushConsts {
	vec4 screenBounds;
	vec4 gradientInfo;
} pc;

layout(binding = 0) uniform sampler2D gradientTexture;

layout(location = 0) out vec4 outColour;

void main() { 
	float gradientPos = (pixelPos.y - (pc.gradientInfo.y + (pc.gradientInfo.z * (pixelPos.x - pc.gradientInfo.x)))) / pc.gradientInfo.w;
	outColour = (frontColour.a * texture (gradientTexture, vec2 (gradientPos, 0.5)));
}