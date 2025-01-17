#version 450

layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D textureSampler;
layout(binding = 1) uniform sampler2D paletteSampler;

void main()
{
	float index = (texture(textureSampler, inTexCoord.xy).x * 255.0 + 0.5) / 256.0;

	outColor = texture(paletteSampler, vec2(index, 0.5));
}