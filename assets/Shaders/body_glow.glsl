#type vertex
#version 330 core

layout(location = 0) in vec2 a_LocalPos;
layout(location = 1) in vec2 a_InstancePos;
layout(location = 2) in vec2 a_InstanceVelocity;
layout(location = 3) in float a_InstanceRadius;

uniform mat4 u_ViewProjection;
uniform float u_RadiusScale;

out vec2 v_LocalPos;
out float v_Speed;

void main()
{
    // Convert [-0.5, 0.5] quad into a [-1, 1] coordinate.
    v_LocalPos = a_LocalPos * 2.0;

    // Radius determines the size of the quad in world space.
    vec2 worldPos = a_InstancePos + a_LocalPos * (a_InstanceRadius * u_RadiusScale * 2.0);

    gl_Position = u_ViewProjection * vec4(worldPos, 0.0, 1.0);

    v_Speed = length(a_InstanceVelocity);
}

#type fragment
#version 330 core

in vec2 v_LocalPos;
in float v_Speed;

uniform float u_MaxSpeed;
uniform float u_GlowIntensity;
uniform vec3 u_ColorTint;

out vec4 FragColor;

void main()
{
    float radiusSquared = dot(v_LocalPos, v_LocalPos);
    if (radiusSquared > 1.0)
        discard;

    float speed = clamp(v_Speed / max(u_MaxSpeed, 0.001), 0.0, 1.0);
    vec3 color = mix(vec3(1.0, 0.22, 0.04),
                     vec3(0.08, 0.48, 1.0), speed) * u_ColorTint;

    // One cheap directional gradient keeps the discs readable as spheres.
    float light = clamp(0.72 - dot(v_LocalPos, vec2(0.22, -0.18)),
                        0.38, 1.0);

    // Analytic edge smoothing is much cheaper than multisampling the frame.
    float edgeWidth = fwidth(radiusSquared);
    float alpha = 1.0 - smoothstep(1.0 - edgeWidth, 1.0, radiusSquared);
    FragColor = vec4(color * light * u_GlowIntensity, alpha);
}
