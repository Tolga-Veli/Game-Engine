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
uniform float u_Time;

out vec4 FragColor;

void main()
{
    float dist = length(v_LocalPos);
    if (dist > 1.0)
        discard;

    float z = sqrt(max(0.0, 1.0 - dist * dist));
    vec3 normal = vec3(v_LocalPos.x, v_LocalPos.y, z);
    float speed = clamp(v_Speed / max(u_MaxSpeed,0.001), 0.0, 1.0);
    vec3 slowColor = vec3(1.0, 0.18, 0.025);
    vec3 fastColor = vec3(0.12, 0.45, 1.0);
    vec3 baseColor = mix(slowColor, fastColor, speed);
    baseColor *= u_ColorTint;

    vec3 lightDir = normalize(vec3(-0.45, 0.35, 1.0));
    float diffuse = max(dot(normal,lightDir), 0.0);
    float lightning = 0.12 + diffuse * 0.88;
    float rim = pow(clamp(1.0 - normal.z,0.0,1.0), 2.5);
    float atmosphere = rim * smoothstep(0.25, 0.85, dist);
    float surfaceFactor = 0.95 + 0.05 * normal.y;
    vec3 surfaceColor = baseColor * lightning * surfaceFactor;
    float core = 1.0 - smoothstep(0.0, 0.62, dist);
    surfaceColor += baseColor * core * 0.35;
    vec3 atmosphereColor = mix(vec3(0.15,0.4,1.0), baseColor, 0.65);
    surfaceColor += atmosphereColor * atmosphere * 0.7 * u_GlowIntensity;
    float glow = pow(1.0 - smoothstep(0.35, 1.0, dist), 2.0);
    surfaceColor *= 1.0 + 0.04 * sin(u_Time * 2.0);
    float alpha = smoothstep(1.0, 0.75, dist);
    alpha = max(alpha, glow * 0.35);
    FragColor = vec4(surfaceColor, alpha);
}
