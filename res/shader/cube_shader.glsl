//shader vertex
#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
    v_texCoord = texCoord;
}


//shader fragment
#version 430 core

in vec2 v_texCoord;

out vec4 FragColor;

uniform float u_percent;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;

void main()
{
    vec4 texColor = mix(texture(u_texture2, v_texCoord), texture(u_texture1, v_texCoord), u_percent);
    FragColor = texColor;
}
