//~VERT SHADER
#version 460 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords   = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}


//~FRAG SHADER
#version 460 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite_texture;
uniform vec3 spriteColor;

vec2 position;

float plot(vec2 st) {    
    return smoothstep(0.2, 0.0, abs(st.y - st.x));
}

void main()
{    
	//RGBA
	vec3 color       = {0.0f, 1.0f, 0.0f};
	
	vec2 st          = gl_FragCoord.xy;
	
	float percentage = plot(st);
	
	color            = percentage * color;
	
    gl_FragColor     = vec4(color, 1.0f);
}

// * texture(sprite_texture, TexCoords) ~ NO TEXTURES

