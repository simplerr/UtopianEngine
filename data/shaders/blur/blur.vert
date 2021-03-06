#version 450

layout (location = 0) in vec3 InPos;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec2 OutUV;

out gl_PerVertex
{
   vec4 gl_Position;
};

void main() 
{
   OutUV = InUV;
   gl_Position = vec4(InPos.xyz, 1.0);
}