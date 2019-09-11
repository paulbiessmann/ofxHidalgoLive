#version 120

uniform float u_time;
uniform sampler2DRect tex0;
uniform sampler2DRect imageMask;
uniform float rand;

varying vec2 texCoordVarying;

void main()
{
    
    
    vec4 texel0 = texture2DRect(tex0, texCoordVarying);
    vec4 texel1 = texture2DRect(imageMask, texCoordVarying);
    
    
    gl_FragColor = vec4(rand - texel0.r, 1.0 - texel0.g, rand, texel0.a);
}
