
cbuffer cbPerObject : register(b0)
{
	float4x4 gLocation;
	float4x4 gRotation;
	float4x4 gScale;
};

cbuffer cbPass : register(b1)
{
	float4x4 gViewProj;
	float gTime;
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float4 Normal : NORMAL;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Normal : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float4x4 gWorld = mul(mul(gScale,gRotation), gLocation);
	float3 PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;

	
	vout.PosH = mul(float4(PosW, 1.0f), gViewProj);

    vout.Normal = mul(vin.Normal, gRotation);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return (pin.Normal + 1) * 0.5 ;
}

