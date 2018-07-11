#version 330 core

in vec2 frag_UV;
in vec4 frag_Color;

out vec4 frag_DrawColor;

uniform sampler2D SpineView_Image;

void main()
{
    frag_DrawColor = texture(SpineView_Image, frag_UV) * frag_Color;
}