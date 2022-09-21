#ifndef _SKYBOX_FX_
#define _SKYBOX_FX_

#include "params.fx"

struct VS_IN
{
    float3 localPos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output = (VS_OUT)0;
    // Translation은 하지않고 Rotation만 적용. w를 0.f로 해서 Translation은 하지 않게.
    float4 viewPos = mul(float4(input.localPos, 0.f), g_matView);
    float4 clipSpacePos = mul(viewPos, g_matProjection); //투영변환하면은 클립스페이스이다. 거기에서 w를 나눠줘야 투영 좌표계이다.

    //래스터라이저 단계로 넘어가기 전에 x/w, y/w, z/w, w/w로 나눠줘서 투영좌표계로 넘어가는데(나눠주는 이유 skybox.txt 참고)
    // z가 깊이고 깊이를 항상 1로 만들기 위해서 z에다가 w를 넣어준다. 그러면 x/w, y/w, w/w, w/w가 되어서 항상 z가 1이 될 것이다.
    output.pos = clipSpacePos.xyww; //z자리에 w를 넣어줬음.
    output.uv = input.uv;

    return output;
}

float4 PS_Main(VS_OUT input) : SV_Target
{
    float4 color = g_tex_0.Sample(g_sam_0, input.uv); //g_tex_0(0번 텍스처)에 있는 것을 받아서 샘플링해줘서 uv값을 이용해서 추출해서 color를 뱉어준다.

    return color;
}

#endif