#type vertex
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uViewProjection;
uniform mat4 uModel;

void main(){
  gl_Position = uViewProjection * uModel * vec4(aPos, 1.0);
}

#type fragment
#version 330 core

out vec4 FragColor;

uniform vec4 uColor;

void main(){
  FragColor = uColor;
}

