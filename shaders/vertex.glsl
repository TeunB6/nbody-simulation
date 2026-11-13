#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aVel;

uniform mat4 view;
uniform mat4 projection;
uniform bool colorBySpeed;

out vec3 vertColor;

vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(0.0, 2.0/3.0, 1.0/3.0)) * 6.0 - 3.0);
    return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = 5.0;

    if (!colorBySpeed) {
        vertColor = vec3(1.0);
        return;
    }

    float s = length(aVel);
    vec3 dir = s > 1e-6 ? aVel / s : vec3(0.0);

    float hue = atan(dir.y, dir.x) / 6.2831853 + 0.5;  // azimuth in xy-plane
    float elev = dir.z;                                // -1 (down) .. +1 (up)

    float sat = 1.0 - 0.7 * max(elev, 0.0);            // moving +z fades toward white
    float val = 1.0 + 0.6 * min(elev, 0.0);            // moving -z darkens
    val *= mix(0.4, 1.0, s / (s + 0.1));               // speed still sets brightness

    vertColor = hsv2rgb(vec3(hue, sat, val));

}
