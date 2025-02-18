#version 450
// Vertex attributes
layout(location = 0) in vec3 vPos;

uniform mat4 _LightSpace;
uniform mat4 _Model;

void main() 
{
	gl_Position = _LightSpace * _Model * vec4(vPos, 1.0);
}