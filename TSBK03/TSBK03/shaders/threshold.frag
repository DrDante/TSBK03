
#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    vec4 in_color = texture(texUnit, outTexCoord);
    in_color = max(in_color - vec4(1.0), vec4(0.0));

    out_Color = in_color;
}
