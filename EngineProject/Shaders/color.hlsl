
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
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Normal : NORMAL;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4x4 gWorld = mul(mul(gScale,gRotation), gLocation);
	//float4x4 gWorld = mul(gLocation, mul(gRotation, gScale));

	float3 PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	//float3 PosW = mul(gWorld, float4(vin.PosL, 1.0f)).xyz;

	vout.PosH = mul(float4(PosW, 1.0f),gViewProj );
	//vout.PosH = mul(gViewProj, float4(PosW, 1.0f));


	vout.TexC = vin.TexC;
    vout.Normal = mul(vin.Normal, gRotation);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 color;
	color = pow((pin.Normal + 1) * 0.5,1 / 2.2f);
	//float4 color = {pin.TexC.x,pin.TexC.y,1.f,1.f};
	return color;
}


