#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outPosition;
layout(location = 1) out vec2 outTexCoord;

void main()
{
	outPosition = inPosition;
	outTexCoord = inTexCoord;
}