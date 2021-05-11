#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform PushConsts {
	vec2 screenSize;
	float textureBounds[4];
	vec2 vOffsetAndScale;
} pc;

layout(location = 0) out vec2 texturePos;

void main() {
	vec2 scaled = inPosition / (0.5 * pc.screenSize.xy);
	gl_Position = vec4 (scaled.x - 1.0, 1.0 - scaled.y, 0, 1.0);
	texturePos = (inPosition - vec2 (pc.textureBounds[0], pc.textureBounds[1])) / vec2 (pc.textureBounds[2], pc.textureBounds[3]);
	texturePos = vec2 (texturePos.x, pc.vOffsetAndScale.x + pc.vOffsetAndScale.y * texturePos.y);
}