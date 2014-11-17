#version 150

in vec2 out_TexCoord;

out vec4 out_Color;

uniform sampler2D texUnit;

void main(void)
{
    out_Color = vec4(vec3(texture(texUnit, out_TexCoord).x), 1.0);
}
