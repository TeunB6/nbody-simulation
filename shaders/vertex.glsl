#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aVel;

out vec3 vertColor;

vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(0.0, 2.0/3.0, 1.0/3.0)) * 6.0 - 3.0);
    return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

void main()
{
    gl_Position = vec4(aPos, 1.0);
    gl_PointSize = 5.0;

    float s = length(aVel);
    float hue = atan(aVel.y, aVel.x) / 6.2831853 + 0.5;   // angle in xy-plane -> [0,1]
    float t = s / (s + 0.1);                               // smaller k = brighter for slow particles
    vertColor = hsv2rgb(vec3(hue, 1.0, mix(0.4, 1.0, t)));
}
