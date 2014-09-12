#version 150

in vec2 out_TexCoord;

out vec4 out_Color;

uniform sampler2D texUnit;

void main(void)
{
    out_Color = texture(texUnit, out_TexCoord);
}