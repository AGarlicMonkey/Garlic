#version 460 core

layout(location = 0) in vec3 Position3D;

layout(std140, binding = 3) uniform modelBuffer {
	mat4 model;
	mat4 normalMatrix;
};

void main(){
	gl_Position = model * vec4(Position3D, 1.0);
}