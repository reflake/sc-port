#version 450

layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D textureSampler;
layout(binding = 1) uniform sampler2D paletteSampler;

void main()
{
  float kEpsilon = 0.0000046039;
	float index = texture(textureSampler, inTexCoord.xy).x - kEpsilon;

	outColor = texture(paletteSampler, vec2(index, 0.0f));
}