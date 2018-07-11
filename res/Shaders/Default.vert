#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

out vec2 frag_UV;
out vec4 frag_Color;

uniform mat4 SpineView_MVP;

void main()
{
    frag_UV     = uv;
    frag_Color  = color;
    gl_Position = SpineView_MVP * vec4(pos, 0.0, 1.0);
}