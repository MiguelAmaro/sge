//~VERT SECTION
#version 330 core

layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x + 0.4f, aPos.y, aPos.z, 1.0);
}


//~FRAG SECTION
#version 330 core

out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 1.0f, 0.2f, 1.0f);
}

