/*
    u8 exe_file_name[MAX_PATH];
    
    DWORD size_of_file_name = GetModuleFileName(0, exe_file_name, sizeof(exe_file_name));
    
    u8 *one_past_last_slash = exe_file_name +  size_of_file_name;
    
    for(u8 *scan = exe_file_name; *scan; ++scan)
    {
        if(*scan == '\\')
        {
            one_past_last_slash = scan + 1;
        }
    }
    
    AllocConsole();
    
    freopen("CONOUT$", "w", stdout);
    
    HANDLE Debug_console = GetStdHandle(STD_OUTPUT_HANDLE );
    */




// TODO: figure out why app crashes if stick is not connected
//This is for the joystick!!!
//CreateDevice(Window, Instance);
/*
                                                                                                                                                                                                                                                        
HDC   gl_device_context = GetDC(Window);
HGLRC gl_render_context = win32_Init_OpenGL(gl_device_context);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
ASSERT(gladLoadGL());
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
u32 gl_major = 0;
u32 gl_minor = 0;
                                                                                                                        
glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
printf       ("OPENGL VERSION: %d.%d \n"  , 
            gl_major, gl_minor);
*/
// NOTE(MIGUEL): This should Init on users command
//win32_serial_Port_Init();



// **************************************
// InitGL STUFF
//
// // TODO(MIGUEL): Find a better system for this shit
// // NOTE(MIGUEL): 02/26/2021 - Ditching GL & following HMH SW Rendering approach for learning
// **************************************
#if RENDER_OPENGL
//~ INIT SPRITE

f32 sprite_vertices[] = { 
    // pos      // tex
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 
    
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f
};

Sprite.vertices = sprite_vertices;

//printf("gen vert arrays addr: %llx#2 ", (u64)&glGenVertexArrays);
//printf("gen vert arrays addr: %llx#2 ", (u64)&glGetError);

OpenGL_VertexBuffer_Create  (&(sprite_render_info.vertex_buffer_id), sprite_vertices, sizeof(sprite_vertices));

GL_Call(glGenVertexArrays(1, &sprite_render_info.vertex_attributes_id));
GL_Call(glBindVertexArray(    sprite_render_info.vertex_attributes_id));

GL_Call(glEnableVertexAttribArray(0));
GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));


// THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
GL_Call(glGenTextures(1, &sprite_render_info.texture));
GL_Call(glBindTexture(GL_TEXTURE_2D, sprite_render_info.texture));
// CONFIGUE OPENGL WRAPPING OPTIONS
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
// CONFIGURE OPENGL FILTERING OPTIONS
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST    ));
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR     ));

//GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 

// LOAD TEXTURE
s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
//stbi_set_flip_vertically_on_load(true);  
u8 *sprite_tex_data = stbi_load("../res/images/geo.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 
if(sprite_tex_data)
{
    //printf("Tex Data: \n %d | %d | %s  \n", sprite_tex_width, sprite_tex_height, sprite_tex_data);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // NOTE(MIGUEL): NO AFFECT
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite_tex_width, sprite_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sprite_tex_data));
    GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
}
else
{
    printf("Failed to load texture");
}

stbi_image_free(sprite_tex_data);


// THIS SHADER MAyBE FUCKED UP
ReadAShaderFile(&sprite_render_info.shader, "../res/shaders/SpriteShader.glsl");


// NOTE(MIGUEL): ISSUES HERE !!!! 
GL_Call(glUseProgram(sprite_render_info.shader));
GL_Call(glUniform1i(glGetUniformLocation(sprite_render_info.shader, "sprite_texture"), 1));

// NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown

// FOR LATER
mat4 projection;
GL_Call(glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f, projection));
GL_Call(glUniformMatrix4fv(glGetUniformLocation(sprite_render_info.shader, "projection"), 1, GL_FALSE, (f32 *)projection));

// UNBIND BUFFER
GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
GL_Call(glBindVertexArray(0));

sprite_render_info.uniform_model      = glGetUniformLocation(sprite_render_info.shader, "model");
sprite_render_info.uniform_color      = glGetUniformLocation(sprite_render_info.shader, "spriteColor");
sprite_render_info.uniform_projection = glGetUniformLocation(sprite_render_info.shader, "projection");


//~ INIT SPRITE NICK

// NOTE(MIGUEL): Use GEO Sprite vertices

Nick.vertices = sprite_vertices;

OpenGL_VertexBuffer_Create(&nick_render_info.vertex_buffer_id, sprite_vertices, sizeof(sprite_vertices));

//GL_Call(glBindBuffer(GL_ARRAY_BUFFER, Nick.vertex_Attributes));
GL_Call(glGenVertexArrays(1, &nick_render_info.vertex_attributes_id));
GL_Call(glBindVertexArray(nick_render_info.vertex_attributes_id));
GL_Call(glEnableVertexAttribArray(0));
GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));


// THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
GL_Call(glGenTextures(1, &nick_render_info.texture));
GL_Call(glBindTexture(GL_TEXTURE_2D, nick_render_info.texture));
// CONFIGUE OPENGL WRAPPING OPTIONS
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_MIRRORED_REPEAT));
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T    , GL_MIRRORED_REPEAT));
// CONFIGURE OPENGL FILTERING OPTIONS
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST        ));
GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR         ));

//GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 

// LOAD TEXTURE
//s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
//stbi_set_flip_vertically_on_load(true);  
sprite_tex_data = stbi_load("../res/images/nick.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 

if(sprite_tex_data)
{
    // NOTE(MIGUEL): NO AFFECT
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite_tex_width, sprite_tex_height, 0, GL_RGBA,      GL_UNSIGNED_BYTE, sprite_tex_data));
    GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
}
else
{
    printf("Failed to load texture");
}

stbi_image_free(sprite_tex_data);


// THIS SHADER MAyBE FUCKED UP
ReadAShaderFile(&nick_render_info.shader, "../res/shaders/Nick.glsl");


// NOTE(MIGUEL): ISSUES HERE !!!! 
GL_Call(glUseProgram(nick_render_info.shader));
GL_Call(glUniform1i(glGetUniformLocation(nick_render_info.shader, "sprite_texture"), 1));

// NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown
d
// FOR LATER
mat4 nick_projection;
GL_Call(glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f, nick_projection));
GL_Call(glUniformMatrix4fv(glGetUniformLocation(nick_render_info.shader, "projection"), 1, GL_FALSE, (f32 *)nick_projection));

// UNBIND BUFFER
GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
GL_Call(glBindVertexArray(0));

nick_render_info.uniform_model      = glGetUniformLocation(nick_render_info.shader, "model"      );
nick_render_info.uniform_color      = glGetUniformLocation(nick_render_info.shader, "spriteColor");
nick_render_info.uniform_input      = glGetUniformLocation(nick_render_info.shader, "mousePos"   );
nick_render_info.uniform_projection = glGetUniformLocation(nick_render_info.shader, "projection" );
#endif


//~ GRAPHICS - HARDWARE - OPENGL
#if RENDER_OPENGL
// NOTE(MIGUEL): 02/26/2021 - NO OPENGL RENDERING YET
local_persist b32 first_render = true;

//glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


// NOTE(MIGUEL): A lot of fuckery going on here
mat4 model                    = GLM_MAT4_ZERO_INIT; // constructor
mat4 nick_model               = GLM_MAT4_ZERO_INIT; // constructor
nick_render_info.matrix_model = (f32 *)&nick_model;

GL_Call(glUseProgram(sprite_render_info.shader));

if(first_render)
{
    glm_translate_make(translation     , (vec3){200.0f, 200.0f, 0.0f});
    glm_scale_make    (scale           , (vec3){200.0f, 200.0f, 1.0f});
    glm_rotate_make   (rotation        , glm_rad(10.0f) ,(vec3){0.0f, 1.0f, 0.0f});
    
    glm_translate_make(nick_translation, (vec3){1.0f, 1.0f, 0.0f});
    glm_scale_make    (nick_scale      , (vec3){200.0f, 200.0f, 1.0f});
    glm_rotate_make   (nick_rotation   , glm_rad(10.0f) ,(vec3){0.0f, 1.0f, 0.0f});
    
    Helpers_Display _Matrix4(translation, "Translate Matrix");
    Helpers_Display_Matrix4(scale      , "Scale Matrix"    );
    Helpers_Display_Matrix4(rotation   , "Rotate Matrix"   );
    
    first_render = false;
}
//rotate++;

//~SPRITE RENDERER_00 BACKGROUND
GL_Call(glUniformMatrix4fv(sprite_render_info.uniform_model, 1, GL_FALSE, (f32 *)model));
GL_Call(glUniform3f       (sprite_render_info.uniform_color, color[0],color[1], color[2]));

GL_Call(glActiveTexture(GL_TEXTURE1));
GL_Call(glBindTexture  (GL_TEXTURE_2D, sprite_render_info.texture));

// Enables the alpha channel
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

GL_Call(glBindVertexArray(sprite_render_info.vertex_attributes_id));
GL_Call(glDrawArrays     (GL_TRIANGLES, 0, 6));
GL_Call(glBindVertexArray(0));



//~SPRITE RENDERER_01
vec3 player_translate;
glm_vec3_copy(((App *)(g_platform.permanent_storage))->player_translate, player_translate);

glm_translate(translation, player_translate);

glm_mat4_mulN((mat4 *[]){&translation, &scale }, 2, model);

//glm_mat4_print(model, stdout);

//Helpers_Display_Matrix4(model, "Model Matrix");

GL_Call(glUniformMatrix4fv(sprite_render_info.uniform_model, 1, GL_FALSE, (f32 *)model));
GL_Call(glUniform3f       (sprite_render_info.uniform_color, color[0],color[1], color[2]));

GL_Call(glActiveTexture(GL_TEXTURE1));
GL_Call(glBindTexture  (GL_TEXTURE_2D, sprite_render_info.texture));

// Enables the alpha channel
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

GL_Call(glBindVertexArray(sp rite_render_info.vertex_attributes_id));
GL_Call(glDrawArrays     (GL_TRIANGLES, 0, 6));
GL_Call(glBindVertexArray(0));


//~ SPRITE RENDERER_02

GL_Call(glUseProgram(nick_render_info.shader));

glm_translate       (nick_translation, (vec3){ ( cos(g_platform.current_time) ), (  sin(g_platform.current_time) ), 0.0f} );
glm_mat4_mulN       ((mat4 *[]){&nick_translation, &nick_scale }, 2, nick_model);

win32_OpenGL_draw_sprite(g_platform, &nick_render_info, (f32 *)&nick_color );

SwapBuffers(gl_device_context);
#endif
