#version 120

uniform float u_time;
uniform float rand;

uniform sampler2DRect tex0;

varying vec2 texCoordVarying;

void main()
{
    
    // get the texture coordinates
    texCoordVarying = gl_MultiTexCoord0.xy;
    
    // copy position so we can work with it.
    vec4 pos = gl_Vertex;
    
    
    //pos.x += u_time *100;
    
    pos.x += rand * 100;

    gl_Position = gl_ModelViewProjectionMatrix * pos;
}
