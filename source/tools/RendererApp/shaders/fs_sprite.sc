$input v_color0, v_texcoord0 

#include <bgfx_shader.sh>

USAMPLER2D(s_texColor,  0);
SAMPLER2D(s_palColor,  1);

void main()
{
     uint paletteIndex = texelFetch(s_texColor, ivec2(v_texcoord0), 0).x;
     if(paletteIndex == uint(0))
        discard;
     gl_FragColor = texelFetch(s_palColor, ivec2(paletteIndex,0),0);
}
