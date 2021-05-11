#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 colour;

layout(push_constant) uniform PushConsts {
	vec4 screenBounds;
} pc;

layout(location = 0) out vec4 frontColour;

void main() {
	frontColour = colour;

	vec2 adjustedPos = position - pc.screenBounds.xy;
	vec2 scaledPos = adjustedPos / pc.screenBounds.zw;
	
	gl_Position = vec4 (scaledPos.x - 1.0, 1.0 - scaledPos.y, 0, 1.0);
}