#include"Shaders.hlsl"

// VSMain
float4 VSMain(float3 pos : POSITION, float3 normal : NORMAL) : SV_POSITION
{
    float3 offset = normal * 0.15f; // ¿Ü°û¼± µÎ²²
    float4 worldPos = mul(float4(pos + offset, 1.0f), g_worldMatrix);
    return mul(worldPos, mul(g_viewMatrix, g_projectionMatrix));
}

// PSMain
float4 PSMain() : SV_Target
{
    return float4(0.0f, 0.0f, 0.0f, 1.0f); // »¡°£ ¿Ü°û¼±
}
