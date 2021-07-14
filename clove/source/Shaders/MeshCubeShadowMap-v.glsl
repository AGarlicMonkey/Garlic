#version 450

#include "Constants.glsl"

layout(std140, set = SET_MESH, binding = 0) uniform Model{
	mat4 model;
	mat4 normalMatrix;
};

layout(std140, push_constant) uniform LightSpace{
	mat4 lightSpaceMatrix;
};

layout(location = 0) in vec3 position;

layout(location = 0) out vec4 vertPos;

void main(){
    vertPos = model * vec4(position, 1.0f);
    gl_Position = lightSpaceMatrix * vertPos;
}