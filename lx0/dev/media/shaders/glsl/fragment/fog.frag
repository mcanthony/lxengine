vec3 fog(vec3 color)
{
    float fogFactor = 1.0 - (gl_FragCoord.z / gl_FragCoord.w) / 400.0f;
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    return mix(color, vec3(0.05, 0.05, 0.07), 1.0 - fogFactor);
}
