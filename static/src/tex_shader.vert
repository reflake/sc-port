#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outPosition;
layout(location = 1) out vec2 outTexCoord;

void main()
{
	gl_Position = vec4(inPosition.xy, 0.0, 1.0);

	outPosition = inPosition;
	outTexCoord = inTexCoord;
}