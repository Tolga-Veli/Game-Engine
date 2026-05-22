#type vertex
#version 330 core
layout (location = 0) in vec3 a_Position; // Matches your BufferLayout
layout (location = 1) in vec3 a_Color;

uniform mat4 u_ViewProjection; // Added
uniform mat4 u_Transform;      // Added

out vec3 v_Color;

void main() {
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    v_Color = a_Color;
}

#type fragment
#version 330 core
layout (location = 0) out vec4 color;

in vec3 v_Color;
uniform vec3 u_BaseColor; 

void main() {
    color = vec4(v_Color * u_BaseColor, 1.0);
}
