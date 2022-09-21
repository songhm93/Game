#ifndef _DEFERRED_FX_
#define _DEFERRED_FX_

#include "params.fx"
#include "utils.fx"

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT; //----------0번 슬롯
    float4 weight : WEIGHT;
    float4 indices : INDICES;

    row_major matrix matWorld : W;
    row_major matrix matWV : WV;
    row_major matrix matWVP : WVP;
    uint instanceID : SV_InstanceID; //---------1번 슬롯
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
    float3 viewPos : POSITION;
    float3 viewNormal : NORMAL;
    float3 viewTangent : TANGENT;
    float3 viewBinormal : BINORMAL;
};

VS_OUT VS_Main(VS_IN input) //버텍스 셰이더에선 아래와 같은 일을 해라. 아웃-인-아웃-인. 여기선 정점 단위들 연산.
{
    VS_OUT output = (VS_OUT)0;

    if (g_int_0 == 1) //인스턴싱이 적용
    {
        if (g_int_1 == 1)
            Skinning(input.pos, input.normal, input.tangent, input.weight, input.indices);
        
        output.pos = mul(float4(input.pos, 1.f), input.matWVP);
        output.uv = input.uv;

        output.viewPos = mul(float4(input.pos, 1.f), input.matWV).xyz;
        output.viewNormal = normalize(mul(float4(input.normal, 0.f), input.matWV).xyz);
        output.viewTangent = normalize(mul(float4(input.tangent, 0.f), input.matWV).xyz);
        output.viewBinormal = normalize(cross(output.viewTangent, output.viewNormal));
    }
    else
    {
        if (g_int_1 == 1)
            Skinning(input.pos, input.normal, input.tangent, input.weight, input.indices);

        //WV(P)까지 곱해지게 되면 투영좌표계로 넘어감.
        output.pos = mul(float4(input.pos, 1.f), g_matWVP); //float4의 마지막(W)에 1.f을 넣으면 행렬을 곱할 때 좌표 개념이 되는 것. 방향성만 추출하고 싶으면 0을 넣는다.
        output.uv = input.uv;

        output.viewPos = mul(float4(input.pos, 1.f), g_matWV).xyz; //얘는 빛 연산을 위해 투영좌표계까지 넘어가지 않고 뷰까지만 딱 끊기위해 WV까지만 곱해줌. input.pos는 로컬기준임.
        output.viewNormal = normalize(mul(float4(input.normal, 0.f), g_matWV).xyz);//view붙은건 view스페이스 기준의 pos좌표, normal좌표
        output.viewTangent = normalize(mul(float4(input.tangent, 0.f), g_matWV).xyz);
        output.viewBinormal = normalize(cross(output.viewTangent, output.viewNormal));//tangent를 먼저 해줘야함. 외적은 연산순서에 따라 나오는 벡터가 방향이 달라져서
        // 좌표계를 맞추기 위해 연산 순서를 잘 해줘야 한다. TNB을 XYZ로 맞추기 위해. X cross Y를 해야 양수로 노말벡터가 나옴.
        //모델링 툴에 따라 Binormal(바이노말, 바이탄젠트)이 반대로 갈 수도 있다. 
    }

    return output;
}

struct PS_OUT
{
    float4 position : SV_Target0; //중간결과물 저장
    float4 normal : SV_Target1; //Shader.cpp에서 _graphicsPipelineDesc.RTVFormats
    float4 color : SV_Target2;
};

PS_OUT PS_Main(VS_OUT input) //forward랑 달라짐. 멀티렌더타겟.
{
    PS_OUT output = (PS_OUT)0;

    float4 color = float4(1.f, 1.f, 1.f, 1.f);
    if (g_tex_on_0 == 1)
        color = g_tex_0.Sample(g_sam_0, input.uv);

    float3 viewNormal = input.viewNormal;
    if (g_tex_on_1 == 1)
    {
        // [0,255] 범위에서  [0,1]로 변환
        float3 tangentSpaceNormal = g_tex_1.Sample(g_sam_0, input.uv).xyz;
        // [0,1] 범위에서 [-1,1]로 변환
        tangentSpaceNormal = (tangentSpaceNormal - 0.5f) * 2.f;
        float3x3 matTBN = { input.viewTangent, input.viewBinormal, input.viewNormal };
        viewNormal = normalize(mul(tangentSpaceNormal, matTBN));
    }

    //3개의 렌더타겟에 정보를 저장한다. 빛연산은 나중에. 이것이 포워드 렌더링과의 차이.
    output.position = float4(input.viewPos.xyz, 0.f); 
    output.normal = float4(viewNormal.xyz, 0.f);
    output.color = color;

    return output;
}

#endif