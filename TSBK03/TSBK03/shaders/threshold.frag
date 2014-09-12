#version 150

in vec2 out_TexCoord;

out vec4 out_Color;

uniform sampler2D texUnit;

void main(void)
{
    vec4 in_color = texture(texUnit, out_TexCoord);
    in_color = max(in_color - vec4(1.0), vec4(0.0));

    out_Color = in_color;
}
