#ifndef _UTILS_FX_
#define _UTILS_FX_

#include "params.fx"

LightColor CalculateLightColor(int lightIndex, float3 viewNormal, float3 viewPos) //index는 몇번째 광원인지
{
    LightColor color = (LightColor)0.f;

    float3 viewLightDir = (float3)0.f;

    float diffuseRatio = 0.f;
    float specularRatio = 0.f;
    float distanceRatio = 1.f;

    if (g_light[lightIndex].lightType == 0)
    {
        // Directional Light   우리는 모든 빛 연산은 view좌표계로 통일한다.
        viewLightDir = normalize(mul(float4(g_light[lightIndex].direction.xyz, 0.f), g_matView).xyz);//direction은 월드 기준이라 matView를 곱해서 뷰 좌표계로 변환 후 정규화
        diffuseRatio = saturate(dot(-viewLightDir, viewNormal));//viewLightDir를 -해서 반대 방향으로 하고 뷰노말이랑 내적. saturate를 하면 0~1 사이로 보정해줌.
        //내적한 값이 음수로 나오고 saturate를 하면 0이 나옴.
    }
    else if (g_light[lightIndex].lightType == 1)
    {
        // Point Light
        float3 viewLightPos = mul(float4(g_light[lightIndex].position.xyz, 1.f), g_matView).xyz;
        viewLightDir = normalize(viewPos - viewLightPos);
        diffuseRatio = saturate(dot(-viewLightDir, viewNormal));

        float dist = distance(viewPos, viewLightPos); //광원과 빛이 닿는 픽셀과의 거리
        if (g_light[lightIndex].range == 0.f)
            distanceRatio = 0.f;
        else //범위내에서 멀어질수록 빛이 약해지게
            distanceRatio = saturate(1.f - pow(dist / g_light[lightIndex].range, 2));
    }
    else
    {
        // Spot Light는 각도 범위내에 있는지 체크, 광원 원점으로부터 최대 거리내에 있는지 체크.
        float3 viewLightPos = mul(float4(g_light[lightIndex].position.xyz, 1.f), g_matView).xyz;
        viewLightDir = normalize(viewPos - viewLightPos);
        diffuseRatio = saturate(dot(-viewLightDir, viewNormal));

        if (g_light[lightIndex].range == 0.f)
            distanceRatio = 0.f;
        else
        {
            float halfAngle = g_light[lightIndex].angle / 2;

            float3 viewLightVec = viewPos - viewLightPos;
            float3 viewCenterLightDir = normalize(mul(float4(g_light[lightIndex].direction.xyz, 0.f), g_matView).xyz); //광원의 정방향 벡터. 방향이니까 float4에서 마지막 0

            float centerDist = dot(viewLightVec, viewCenterLightDir); //내적으로 광원의 길이를 구한다. viewCenterLightDir은 단위벡터.
            //viewLightVec에서 viewCenterLightDir로 직교 투영투영된 길이.즉, 광원의 길이(centerDist)를 구하려면 viewLightVec과 광원 정방향 벡터(단위벡터)와 내적하면 된다.
            //a dot b = a크기 곱하기 b크기 코사인세타이다. a크기가 그대로 viewLightVec이고, b크기는 단위벡터이기 때문에 결과적으로 a크기 곱하기 코사인세타가 된다.
            //dot함수에서 내부적으로 계산해주는 듯. a,b내적으로 a에 투영된 길이 구하기.jpg 참고
            //그려놓은 그림은 투영된 길이가 최대 거리를 벗어난 케이스.

            distanceRatio = saturate(1.f - centerDist / g_light[lightIndex].range);

            float lightAngle = acos(dot(normalize(viewLightVec), viewCenterLightDir)); //내적을 통해 각도 구함.
            // 두 벡터 모두 정규화해서 내적을 한 결과값을 acos하면 각도가 나온다.

            if (centerDist < 0.f || centerDist > g_light[lightIndex].range) // 최대 거리를 벗어났는지
                distanceRatio = 0.f;
            else if (lightAngle > halfAngle) // 최대 시야각을 벗어났는지
                distanceRatio = 0.f;
            else // 거리에 따라 적절히 세기를 조절
                distanceRatio = saturate(1.f - pow(centerDist / g_light[lightIndex].range, 2));
        }
    }

    float3 reflectionDir = normalize(viewLightDir + 2 * (saturate(dot(-viewLightDir, viewNormal)) * viewNormal));
    float3 eyeDir = normalize(viewPos);
    specularRatio = saturate(dot(-eyeDir, reflectionDir));
    specularRatio = pow(specularRatio, 2); //제곱인데 더 큰수를 곱하면 값이 작아짐.-> 정반사율이 적어짐
    //(많이 제곱해줄수록 값이 작아지고 빛이 더 중앙으로 몰린다.)
    //(적게 제곱해줄수록 값이 커지고 더 넓게 퍼진다.)

    color.diffuse = g_light[lightIndex].color.diffuse * diffuseRatio * distanceRatio;
    color.ambient = g_light[lightIndex].color.ambient * distanceRatio;
    color.specular = g_light[lightIndex].color.specular * specularRatio * distanceRatio;

    return color;
}

float Rand(float2 co)
{
    return 0.5 + (frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453)) * 0.5; //frac : 소수점 자리를 뽑음. 0~1사이로 나옴.
}

float CalculateTessLevel(float3 cameraWorldPos, float3 patchPos, float min, float max, float maxLv)
{
    float distance = length(patchPos - cameraWorldPos);

    if (distance < min)
        return maxLv;
    if (distance > max)
        return 1.f;

    float ratio = (distance - min) / (max - min);
    float level = (maxLv - 1.f) * (1.f - ratio);
    return level;
}

struct SkinningInfo
{
    float3 pos;
    float3 normal;
    float3 tangent;
};

void Skinning(inout float3 pos, inout float3 normal, inout float3 tangent,
    inout float4 weight, inout float4 indices)
{
    SkinningInfo info = (SkinningInfo)0.f;

    for (int i = 0; i < 4; ++i)
    {
        if (weight[i] == 0.f)
            continue;

        int boneIdx = indices[i];
        matrix matBone = g_mat_bone[boneIdx];

        info.pos += (mul(float4(pos, 1.f), matBone) * weight[i]).xyz;
        info.normal += (mul(float4(normal, 0.f), matBone) * weight[i]).xyz;
        info.tangent += (mul(float4(tangent, 0.f), matBone) * weight[i]).xyz;
    }

    pos = info.pos;
    tangent = normalize(info.tangent);
    normal = normalize(info.normal);
}


#endif