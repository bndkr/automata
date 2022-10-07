#ifndef UTILS_LOAD_TEXTURE_FROM_DATA
#define UTILS_LOAD_TEXTURE_FROM_DATA

#include <d3d11.h>
namespace automata
{
    bool LoadTextureFromData(unsigned char* image_data, ID3D11ShaderResourceView** out_srv, ID3D11Device* pd3dDevice, int width, int height);
}

#endif