#version 150

in vec3 in_Position;
out vec4 lightSourceCoord;

uniform mat4 MTWMatrix;	// Model-to-world-matris.
uniform mat4 WTVMatrix;	// World-to-view-matris (kamera).
uniform mat4 VTPMatrix;	// View-to-projection-matris (lins).

void main(void)
{
	gl_Position = VTPMatrix * WTVMatrix * MTWMatrix * vec4(in_Position, 1.0);
}
