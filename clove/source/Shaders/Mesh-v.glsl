#version 450

#include "Constants.glsl"

layout(std140, set = 0, binding = 0) uniform Model{
	mat4 model;
	mat4 normalMatrix;
};

layout(std140, set = 1, binding = 0) uniform ViewProj{
	mat4 view;
	mat4 proj;
};

layout(std140, set = 2, binding = 1) uniform NumLights{
	int numDirLights;
	int numPointLights;
};
layout(std140, set = 2, binding = 2) uniform LightMatrix{
	mat4 lightSpaceMatrices[MAX_LIGHTS];
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 colour;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 vertPos;
layout(location = 3) out vec3 vertNorm;
layout(location = 4) out vec4 vertPosLightSpaces[MAX_LIGHTS];

void main(){
	gl_Position = proj * view * model * vec4(position, 1.0f);
	
	fragColour = colour;
	fragTexCoord = texCoord;

	vertPos = vec3(model * vec4(position, 1.0f));
	vertNorm = mat3(normalMatrix) * normal;

	for(int i = 0; i < numDirLights; ++i){
		vertPosLightSpaces[i] = lightSpaceMatrices[i] * vec4(vertPos, 1.0f);
	}
}