#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 vertColor;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    gl_PointSize = 3.0;
    vertColor = aPos * 0.5 + 0.5; 
}