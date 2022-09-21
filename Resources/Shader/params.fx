#ifndef _PARAMS_FX_
#define _PARAMS_FX_

struct LightColor 
{
    float4 diffuse;
    float4 ambient;
    float4 specular;
};
//Light.h에 선언한 구조체들과 똑같은 형태
struct LightInfo
{
    LightColor color;
    float4 position;
    float4 direction;
    int lightType;
    float range;
    float angle;
    int padding;
};

cbuffer GLOBAL_PARAMS : register(b0)
{
    int g_lightCount;
    float3 g_lightPadding;
    LightInfo g_light[50];
}


cbuffer TRANSFORM_PARAMS : register(b1) //cbuffer : constant buffer, transform 정보를 받음.
{
    row_major matrix g_matWorld;
    row_major matrix g_matView;
    row_major matrix g_matProjection;
    row_major matrix g_matWV;
    row_major matrix g_matWVP; //row_major : dx와 셰이더가 행렬 접근방식이 달라서 그것을 dx기준으로 맞추기 위함. row->행
    row_major matrix g_matViewInv;
};

cbuffer MARTERIAL_PARAMS : register(b2) //머티리얼 정보를 받음.
{
    int g_int_0;
    int g_int_1;
    int g_int_2;
    int g_int_3;
    float g_float_0;
    float g_float_1;
    float g_float_2;
    float g_float_3;
    int g_tex_on_0; //텍스쳐가 있는지 없는지를 체크하기 위함. 셰이더 언어에선 null체크를 할 수 없어서 이렇게..
    int g_tex_on_1; //0이면 사용안하는거, 1이면 사용하는거
    int g_tex_on_2;
    int g_tex_on_3;
    float2 g_vec2_0;
    float2 g_vec2_1;
    float2 g_vec2_2;
    float2 g_vec2_3;
    float4 g_vec4_0;
    float4 g_vec4_1;
    float4 g_vec4_2;
    float4 g_vec4_3;
    row_major float4x4 g_mat_0;
    row_major float4x4 g_mat_1;
    row_major float4x4 g_mat_2;
    row_major float4x4 g_mat_3;
};

//t0에서 받은 애를 tex_0라고 부른다. ShaderResource가 얘네들이네.
Texture2D g_tex_0 : register(t0);//루트서명에는 b0~b4, t0~t9를 사용할 것이라고 디스크립터 범위를 잡았는데 다 사용하지는 않아도 된다.
Texture2D g_tex_1 : register(t1);
Texture2D g_tex_2 : register(t2);
Texture2D g_tex_3 : register(t3);
Texture2D g_tex_4 : register(t4);

StructuredBuffer<Matrix> g_mat_bone : register(t7);

SamplerState g_sam_0 : register(s0);

#endif