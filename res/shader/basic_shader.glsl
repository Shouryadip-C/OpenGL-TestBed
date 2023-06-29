//shader vertex
#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 texCoord;

out vec3 v_ourColor;
out vec2 v_texCoord;

uniform mat4 u_transform;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_transform * vec4(aPos, 1.0);
    v_ourColor = aColor;
    v_texCoord = texCoord;
}


//shader fragment
#version 430 core

in vec3 v_ourColor;
in vec2 v_texCoord;

out vec4 FragColor;

// uniform vec4 u_viewCoord;
uniform float u_percent;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;

void main()
{
    FragColor = vec4(v_ourColor, 1.0);

    // textures
    vec4 texColor = mix(texture(u_texture2, v_texCoord), texture(u_texture1, v_texCoord), u_percent) * FragColor;
    // texColor = mix(texture(u_texture2, u_viewCoord), texture(u_texture1, u_viewCoord), u_percent);
    FragColor = texColor;
}
