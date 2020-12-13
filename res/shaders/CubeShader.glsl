//~VERT SECTION
#version 330 core

layout (location = 0) in vec3 CubePos;
layout (location = 1) in vec3 CubeColorData;

out vec3 CubeColors;
uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(CubePos, 1.0f);
	CubeColors = CubeColorData;
	
}


//~FRAG SECTION
#version 330 core

out vec4 CubeFragColor;
in vec3 CubeColors;

void main()
{
    CubeFragColor = vec4(CubeColors, 1.0f);
}

