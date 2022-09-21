#ifndef _LIGHTING_FX_
#define _LIGHTING_FX_

#include "params.fx"
#include "utils.fx"

struct VS_IN 
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

struct PS_OUT
{
    float4 diffuse : SV_Target0; //중간 결과물 저장
    float4 specular : SV_Target1;
};

// [Directional Light]
// g_int_0 : Light index. b2 머티리얼 파람
// g_tex_0 : Position 렌더타겟. t0
// g_tex_1 : Normal 렌더타겟. t1
// g_tex_2 : Shadow 렌더타겟. t2
// g_mat_0 : ShadowCamera VP. b2 머티리얼 파람
// Mesh : Rectangle

VS_OUT VS_DirLight(VS_IN input)
{
    VS_OUT output = (VS_OUT)0;

    output.pos = float4(input.pos * 2.f, 1.f);
    output.uv = input.uv;

    return output;
}

PS_OUT PS_DirLight(VS_OUT input)
{
    PS_OUT output = (PS_OUT)0;

    float3 viewPos = g_tex_0.Sample(g_sam_0, input.uv).xyz;  //물체 위치. g_tex_0.Sample(g_sam_0, input.uv);를 한거랑 뒤에 .xyz를 한거에 따라 뭘 뱉는지가 달라짐.
    if (viewPos.z <= 0.f) //viewPos가 뷰스페이스에서의 물체 포지션이다. 뷰스페이스면 카메라가 기준이고 카메라 기준에서 z가 0이거나 0보다 작으면 카메라의 뒤에 있다는 뜻이다.
        clip(-1); //hlsl 내장함수. 음수를 넣으면 return됨.

    float3 viewNormal = g_tex_1.Sample(g_sam_0, input.uv).xyz;

    LightColor color = CalculateLightColor(g_int_0, viewNormal, viewPos);

    // 그림자
    if (length(color.diffuse) != 0) // 난반사가 없다는건 빛이 없는거.
    {
        matrix shadowCameraVP = g_mat_0; //광원위치에서의 카메라의 ViewProjection행렬

        float4 worldPos = mul(float4(viewPos.xyz, 1.f), g_matViewInv);
        float4 shadowClipPos = mul(worldPos, shadowCameraVP); //WVP를 곱해주는셈. 광원위치에서의 카메라를 기준으로한 ClipPosition이 됨.
        float depth = shadowClipPos.z / shadowClipPos.w; //광원위치에서 봤을 때 깊이값.

        //클립 -> 투영좌표계(w로 나눠주기)
        float2 uv = shadowClipPos.xy / shadowClipPos.w;
        // 투영좌표계 -> uv 좌표계
        // x [-1 ~ 1] -> u [0 ~ 1]
        // y [1 ~ -1] -> v [0 ~ 1]
        uv.y = -uv.y;
        uv = uv * 0.5 + 0.5;

        if (0 < uv.x && uv.x < 1 && 0 < uv.y && uv.y < 1)
        {
            float shadowDepth = g_tex_2.Sample(g_sam_0, uv).x; //shadow 쉐이더, PS에서 SV_Target으로 넘겨준 텍스쳐가 g_tex2. x에 담아서 넘겨줬으니 x로 빼낸다.
            if (shadowDepth > 0 && depth > shadowDepth + 0.00001f)
            {
                color.diffuse *= 0.5f;
                color.specular = (float4) 0.f;
            }
        }
    }

    output.diffuse = color.diffuse + color.ambient;
    output.specular = color.specular;

    return output;
}

// [Point Light]
// g_int_0 : Light index
// g_tex_0 : Position RT. t0
// g_tex_1 : Normal RT. t1
// g_vec2_0 : RenderTarget Resolution. 렌더타겟 해상도
// Mesh : Sphere

VS_OUT VS_PointLight(VS_IN input)
{
    VS_OUT output = (VS_OUT)0;

    output.pos = mul(float4(input.pos, 1.f), g_matWVP);//output.pos는 SV_Position.얘는 클립스페이스 형태(-1~1) 곧이곧대로 아래 PS로 넘어가는게 아니라 아래 주석처럼 Screen 좌표(픽셀좌표)이다.
    output.uv = input.uv;

    return output;
}

PS_OUT PS_PointLight(VS_OUT input)
{
    PS_OUT output = (PS_OUT)0;

    // input.pos = SV_Position = Screen 좌표. x는 0~800 y는 0~600
    float2 uv = float2(input.pos.x / g_vec2_0.x, input.pos.y / g_vec2_0.y);//픽셀좌표를 uv좌표로 변환해야함.그래서 x요소는 x로 나눠주고 y는 y로 나눠줌.
    float3 viewPos = g_tex_0.Sample(g_sam_0, uv).xyz; //물체 위치. 똑같이 맨뒤에 xyz가 붙음.
    if (viewPos.z <= 0.f)
        clip(-1);

    int lightIndex = g_int_0;
    float3 viewLightPos = mul(float4(g_light[lightIndex].position.xyz, 1.f), g_matView).xyz; //g_light[lightIndex].position은 월드좌표라서 view좌표로 변환.
    float distance = length(viewPos - viewLightPos);//광원과 물체의 거리를 구함.
    if (distance > g_light[lightIndex].range) //거리가 광원의 범위보다 크면 return
        clip(-1);

    float3 viewNormal = g_tex_1.Sample(g_sam_0, uv).xyz; //normal 추출

    LightColor color = CalculateLightColor(g_int_0, viewNormal, viewPos);

    output.diffuse = color.diffuse + color.ambient;
    output.specular = color.specular;

    return output;
}

// [Final]
// g_tex_0 : Diffuse Color Target. t0
// g_tex_1 : Diffuse Light Target. t1
// g_tex_2 : Specular Light Target. t2
// Mesh : Rectangle

VS_OUT VS_Final(VS_IN input) //내용은 DirectionalLight와 같다.
{
    VS_OUT output = (VS_OUT)0;

    output.pos = float4(input.pos * 2.f, 1.f);
    output.uv = input.uv;

    return output;
}

float4 PS_Final(VS_OUT input) : SV_Target
{
    float4 output = (float4)0;

    float4 lightPower = g_tex_1.Sample(g_sam_0, input.uv); //Diffuse Light 빛을 안받는 곳이면 return.
    if (lightPower.x == 0.f && lightPower.y == 0.f && lightPower.z == 0.f)
        clip(-1);

    //빛을 받는곳이면 컬러와 specular 받아서 빛연산
    float4 color = g_tex_0.Sample(g_sam_0, input.uv);
    float4 specular = g_tex_2.Sample(g_sam_0, input.uv);

    output = (color * lightPower) + specular;
    return output;
}

#endif