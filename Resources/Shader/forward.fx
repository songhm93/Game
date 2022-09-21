#ifndef _DEFAULT_FX_
#define _DEFAULT_FX_

#include "params.fx"
#include "utils.fx"

//콜론 뒤에 오는 것들이 시멘틱.
//이 변수를 셰이더와 연결할 때 사용됨. 모든 변수에 시멘틱이 붙어야 한다.
//Shader.cpp에서 설정해줌
struct VS_IN //버텍스셰이더로 넣어주는 것(매개변수)
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};
//SV SystemValue 붙어있는건 예약된 이름
struct VS_OUT //버텍스셰이더에서 나가는 것(반환형)
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
    float3 viewPos : POSITION;
    float3 viewNormal : NORMAL;
    float3 viewTangent : TANGENT;
    float3 viewBinormal : BINORMAL;
};

VS_OUT VS_Main(VS_IN input) //여기선 정점 단위들 연산. 픽셀셰이더에서 사용할 뷰 스페이스로 변환해주는 일을 한다.
{
    VS_OUT output = (VS_OUT)0;
    //WV(P)까지 곱해지게 되면 투영좌표계로 넘어감.
    output.pos = mul(float4(input.pos, 1.f), g_matWVP); //float4의 마지막(W)에 1.f을 넣으면 행렬을 곱할 때 좌표 개념이 되는 것. 방향성만 추출하고 싶으면 0을 넣는다.
    output.uv = input.uv;

    output.viewPos = mul(float4(input.pos, 1.f), g_matWV).xyz; //얘는 빛 연산을 위해 투영좌표계까지 넘어가지 않고 뷰까지만 딱 끊기위해 WV까지만 곱해줌. input.pos는 로컬기준임.
    output.viewNormal = normalize(mul(float4(input.normal, 0.f), g_matWV).xyz);//view붙은건 view스페이스 기준의 pos좌표, normal좌표
    output.viewTangent = normalize(mul(float4(input.tangent, 0.f), g_matWV).xyz);
    output.viewBinormal = normalize(cross(output.viewTangent, output.viewNormal));//tangent를 먼저 해줘야함. 외적은 연산순서에 따라 나오는 벡터가 방향이 달라져서
    // 좌표계를 맞추기 위해 연산 순서를 잘 해줘야 한다. TNB을 XYZ로 맞추기 위해. X cross Y를 해야 양수로 노말벡터가 나옴.
    //모델링 툴에 따라 Binormal(바이노말, 바이탄젠트)이 반대로 갈 수도 있다. 

    return output;
}

float4 PS_Main(VS_OUT input) : SV_Target//픽셀 셰이더에선 아래와 같은 일을 해라. 픽셀 단위들 작업.
{
    float4 color = float4(1.f, 1.f, 1.f, 1.f);
    if (g_tex_on_0) //기본 흰색으로 하고 만약 텍스쳐를 설정했다면(텍스쳐가 있다면) 색상을 텍스쳐로. 
        color = g_tex_0.Sample(g_sam_0, input.uv); //텍스처를 받아서 색상 설정
     //g_tex_0(0번 텍스처)에 있는 것을 받아서 샘플링해줘서 uv값을 이용해서 추출해서 color를 뱉어준다.


    float3 viewNormal = input.viewNormal;
    //노말맵을 사용할 때. 노말을 탄젠트 스페이스 기준이었던것을 뷰 스페이스로 변환해줘야 한다.
    if (g_tex_on_1)
    {
        // [0,255] 범위에서 [0,1]로 변환. 정수값을 원래 색상값인 0~255로 사용했지만 그것을 여기서는 방향벡터로 해석할것이기 때문에 -1~1로 변환해줘야 한다. 여기서 먼저 0~1로 변환.
        float3 tangentSpaceNormal = g_tex_1.Sample(g_sam_0, input.uv).xyz;
        // [0,1] 범위에서 [-1,1]로 변환
        tangentSpaceNormal = (tangentSpaceNormal - 0.5f) * 2.f;
        float3x3 matTBN = { input.viewTangent, input.viewBinormal, input.viewNormal };
        viewNormal = normalize(mul(tangentSpaceNormal, matTBN));
    }

    LightColor totalColor = (LightColor)0.f;
    //들고있는 라이트들을 순회하면서 라이트들이 들고 있는 정보로 색상 연산
    for (int i = 0; i < g_lightCount; ++i)
    {
         LightColor color = CalculateLightColor(i, viewNormal, input.viewPos); //CalculateLightColor함수는 utils.hlsli에 정의되어있음.
         totalColor.diffuse += color.diffuse;
         totalColor.ambient += color.ambient;
         totalColor.specular += color.specular;
    }

    color.xyz = (totalColor.diffuse.xyz * color.xyz)  //모든 빛에 의해 변한 물체의 색상
        + totalColor.ambient.xyz * color.xyz
        + totalColor.specular.xyz;

     return color;
}

// [Texture Shader] 라이팅 영향을 받지않고 그대로 그리기 위한 용도. UI그릴때 사용.
// g_tex_0 : Output Texture
// AlphaBlend : true
struct VS_TEX_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_TEX_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

VS_TEX_OUT VS_Tex(VS_TEX_IN input)
{
    VS_TEX_OUT output = (VS_TEX_OUT)0;

    output.pos = mul(float4(input.pos, 1.f), g_matWVP);
    output.uv = input.uv;

    return output;
}

float4 PS_Tex(VS_TEX_OUT input) : SV_Target
{
    float4 color = float4(1.f, 1.f, 1.f, 1.f);
    if (g_tex_on_0)
        color = g_tex_0.Sample(g_sam_0, input.uv);

    return color;
}





#endif