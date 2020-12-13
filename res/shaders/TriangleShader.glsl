//~VERT SECTION
#version 330 core

layout (location = 0) in vec3 triPos;
layout (location = 1) in vec3 ColorData;

uniform vec4 trianglePosOffset;
out vec3 Colors;

void main()
{
    gl_Position = vec4(triPos.x + trianglePosOffset.x, triPos.y + trianglePosOffset.y, triPos.z + trianglePosOffset.z, 1.0);
	Colors = ColorData;
}


//~FRAG SECTION
#version 330 core

uniform vec4 triangleColorOffset; // we set this variable in the OpenGL code.
out vec4 FragColor;
in vec3 Colors;

void main()
{
    FragColor = vec4(Colors.x + triangleColorOffset.x, Colors.y + triangleColorOffset.y, Colors.z * triangleColorOffset.z, 1.0f);
}

