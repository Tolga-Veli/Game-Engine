#type vertex
#version 460 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in float a_Radius;
layout(location = 2) in vec3 a_Color;

uniform mat4 u_ViewProjection;

out vec3 v_Color;

void main()
{
    gl_Position = u_ViewProjection * vec4(a_Position, 0.0, 1.0);
    gl_PointSize = a_Radius * 2.0;
    v_Color = a_Color;
}

#type fragment
#version 460 core

in vec3 v_Color;

out vec4 FragColor;

void main()
{
    vec2 p = gl_PointCoord - vec2(0.5);
    if (length(p) > 0.5)
        discard;
    FragColor = vec4(v_Color, 1.0);
}
