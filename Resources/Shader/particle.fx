#ifndef _PARTICLE_FX_
#define _PARTICLE_FX_

#include "params.fx"
#include "utils.fx"

struct Particle
{
    float3  worldPos;
    float   curTime; //경과시간
    float3  worldDir;
    float   lifeTime;
    int     alive; //0이면 죽음(렌더X) 1이면 살음(렌더링)
    float3  padding;
};

StructuredBuffer<Particle> g_data : register(t9); //(u0 컴퓨트셰이더, CS_Main)에서 연산한것과 맵핑될 것. 얘가 그것을 토대로 그려줄 것임.
//GS에서 그린다.

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    uint id : SV_InstanceID;
};

struct VS_OUT
{
    float4 viewPos : POSITION;
    float2 uv : TEXCOORD;
    float id : ID;
};

// VS_MAIN
// g_float_0    : Start Scale
// g_float_1    : End Scale
// g_tex_0      : Particle Texture

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output = (VS_OUT)0.f;

    float3 worldPos = mul(float4(input.pos, 1.f), g_matWorld).xyz;
    worldPos += g_data[input.id].worldPos;

    output.viewPos = mul(float4(worldPos, 1.f), g_matView);
    output.uv = input.uv;
    output.id = input.id;

    return output;
}

struct GS_OUT //정점을 추가해주거나 아예 그리지 않거나
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    uint id : SV_InstanceID;
};

[maxvertexcount(6)] //지오메트리는 t9 레지스터로 데이터 받음.
void GS_Main(point VS_OUT input[1], inout TriangleStream<GS_OUT> outputStream) //여기가 Geometry Shader. point (정점) 하나를 받는다. 점을 받는것
{                                       //outputStream에 그려줘야할 정점이면 정점을 만들어서 넣어주고, 아니면 Cut
    GS_OUT output[4] =
    {
        (GS_OUT)0.f, (GS_OUT)0.f, (GS_OUT)0.f, (GS_OUT)0.f
    };

    VS_OUT vtx = input[0];
    uint id = (uint)vtx.id;
    if (0 == g_data[id].alive) //죽은거면
        return;

    float ratio = g_data[id].curTime / g_data[id].lifeTime;
    float scale = ((g_float_1 - g_float_0) * ratio + g_float_0) / 2.f;

    // View Space
    output[0].position = vtx.viewPos + float4(-scale, scale, 0.f, 0.f);
    output[1].position = vtx.viewPos + float4(scale, scale, 0.f, 0.f);
    output[2].position = vtx.viewPos + float4(scale, -scale, 0.f, 0.f);
    output[3].position = vtx.viewPos + float4(-scale, -scale, 0.f, 0.f);

    // Projection Space
    output[0].position = mul(output[0].position, g_matProjection);
    output[1].position = mul(output[1].position, g_matProjection);
    output[2].position = mul(output[2].position, g_matProjection);
    output[3].position = mul(output[3].position, g_matProjection);

    output[0].uv = float2(0.f, 0.f);
    output[1].uv = float2(1.f, 0.f);
    output[2].uv = float2(1.f, 1.f);
    output[3].uv = float2(0.f, 1.f);

    output[0].id = id;
    output[1].id = id;
    output[2].id = id;
    output[3].id = id;

    outputStream.Append(output[0]);
    outputStream.Append(output[1]);
    outputStream.Append(output[2]);
    outputStream.RestartStrip();

    outputStream.Append(output[0]);
    outputStream.Append(output[2]);
    outputStream.Append(output[3]);
    outputStream.RestartStrip();

    //삼각형 2개를 만든것. 이렇게 만들면 아래 픽셀 셰이더에서 GS_OUT타입으로 받는다.
}

float4 PS_Main(GS_OUT input) : SV_Target
{
    return g_tex_0.Sample(g_sam_0, input.uv); //갖고있었던 버블텍스쳐를 샘플링해서 그려줄것
}

struct ComputeShared
{
    int addCount;
    float3 padding;
};

RWStructuredBuffer<Particle> g_particle : register(u0); //g_particle 배열이라고 생각하면 됨.
RWStructuredBuffer<ComputeShared> g_shared : register(u1); //g_shared 크기가 1인 배열이라고 생각. 공용으로 사용함.

// CS_Main
// g_vec2_1 : DeltaTime / AccTime   AccTime:누적시간. Rand를 위한 변수
// g_int_0  : Particle Max Count
// g_int_1  : AddCount
// g_vec4_0 : MinLifeTime / MaxLifeTime / MinSpeed / MaxSpeed
[numthreads(1024, 1, 1)]
void CS_Main(int3 threadIndex : SV_DispatchThreadID)
{
    if (threadIndex.x >= g_int_0)
        return;

    int maxCount = g_int_0;
    int addCount = g_int_1;
    int frameNumber = g_int_2;
    float deltaTime = g_vec2_1.x;
    float accTime = g_vec2_1.y;
    float minLifeTime = g_vec4_0.x;
    float maxLifeTime = g_vec4_0.y;
    float minSpeed = g_vec4_0.z;
    float maxSpeed = g_vec4_0.w;

    g_shared[0].addCount = addCount; //공용버퍼. 부활해야하는 갯수
    GroupMemoryBarrierWithGroupSync(); //동기화기법. Barrier.기다리게 만든다.

    if (g_particle[threadIndex.x].alive == 0)
    {
        while (true) //lockfree
        {
            int remaining = g_shared[0].addCount;
            if (remaining <= 0)
                break;

            int expected = remaining; //부활 가능한 숫자
            int desired = remaining - 1; //내가 만약 부활이 된다면 남은 부활 숫자. 만약 부활이 된다면이라는것은 안될수도 있다는 것이다.
            int originalValue;
            InterlockedCompareExchange(g_shared[0].addCount, expected, desired, originalValue); //이 함수는 한번에 한번만 실행한다.
            //expected이 g_shared[0].addCount랑 같으면 desired를 g_shared[0].addCount에 넣는다.
            //그리고 g_shared[0].addCount를 originalValue에 넣는다. 이 내용이 한번에 실행된다. 순차적으로 실행이 아니라.
           
            if (originalValue == expected) //부활할 수 있음
            {
                g_particle[threadIndex.x].alive = 1;
                break;
            }
        }

        if (g_particle[threadIndex.x].alive == 1)
        {
            float x = ((float)threadIndex.x / (float)maxCount) + accTime;

            float r1 = Rand(float2(x, accTime));
            float r2 = Rand(float2(x * accTime, accTime));
            float r3 = Rand(float2(x * accTime * accTime, accTime * accTime));
            //랜덤값 추출하는데 noise texture를 load해서 받아온 다음에 uv값으로 랜덤을 추출할 수도 있다.

            // [0.5~1] -> [0~1]로 변환
            float3 noise =
            {
                2 * r1 - 1,
                2 * r2 - 1,
                2 * r3 - 1
            };

            // [0~1] -> [-1~1]로 변환
            float3 dir = (noise - 0.5f) * 2.f;

            g_particle[threadIndex.x].worldDir = normalize(dir);
            g_particle[threadIndex.x].worldPos = (noise.xyz - 0.5f) * 25;
            g_particle[threadIndex.x].lifeTime = ((maxLifeTime - minLifeTime) * noise.x) + minLifeTime;
            g_particle[threadIndex.x].curTime = 0.f;
        }
    }
    else //이미 살아있는 애
    {
        g_particle[threadIndex.x].curTime += deltaTime;
        if (g_particle[threadIndex.x].lifeTime < g_particle[threadIndex.x].curTime)
        {
            g_particle[threadIndex.x].alive = 0;
            return;
        }

        float ratio = g_particle[threadIndex.x].curTime / g_particle[threadIndex.x].lifeTime;
        float speed = (maxSpeed - minSpeed) * ratio + minSpeed;
        g_particle[threadIndex.x].worldPos += g_particle[threadIndex.x].worldDir * speed * deltaTime;
    }
}

#endif