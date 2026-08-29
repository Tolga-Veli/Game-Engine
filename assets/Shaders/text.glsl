#type vertex
#version 450 core

layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec2 a_UV;

uniform mat4 u_ViewProjection;

out vec2 v_UV;

void main() {
  v_UV = a_UV;
  gl_Position = u_ViewProjection * vec4(a_Pos, 0.0, 1.0);
}

#type fragment
#version 450 core

in vec2 v_UV;

uniform sampler2D u_Atlas;
uniform vec3 u_TextColor;

layout(location = 0) out vec4 FragColor;

void main() {
  float alpha = texture(u_Atlas, v_UV).r;
  FragColor = vec4(u_TextColor, alpha);
}

