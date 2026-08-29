#type vertex
#version 330 core

layout(location = 0) in vec2 a_LocalPos;
layout(location = 1) in vec2 a_InstancePos;
layout(location = 2) in float a_InstanceRadius;
layout(location = 3) in float a_InstanceSpeed;

uniform mat4 u_ViewProjection;

void main()
{
    vec2 worldPos = a_InstancePos + a_LocalPos * 100.0;

    gl_Position = u_ViewProjection * vec4(worldPos, 0.0, 1.0);
}

#type fragment
#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
