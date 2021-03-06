#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#define MAX_LIGHTS 16

#include "Light.hlsl"

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNormalMap : register(t1);
Texture2D    gShadowMap : register(t2);

SamplerState gSamPointWrap : register(s0);
SamplerState gSamPointClamp : register(s1);
SamplerState gSamLinearWarp : register(s2);
SamplerState gSamLinearClamp : register(s3);
SamplerState gSamAnisotropicWarp : register(s4);
SamplerState gSamAnisotropicClamp : register(s5);
SamplerComparisonState gSamShadow : register(s6);

cbuffer cbPerObject : register(b0)
{
	float4x4 gLocation;
	float4x4 gRotation;
	float4x4 gScale;
};

cbuffer cbPass : register(b1)
{
	Light gLights[MAX_LIGHTS];
	float4x4 gViewProj;
	float4x4 gPosiProj;

	float4 gAmbientLight;

	float3 gEyePosW;
	float gTime;

	float4x4 gShadowTransform;
};

cbuffer cbMaterial : register(b2)
{
	float4x4 gMatTransform;
	float4 gDiffuseAlbedo;
	float gRoughness;
	float3 gFresnelR0;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float4 Normal : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float3 PosW  : POSITION0;
	float4 ShadowPosH : POSITION1;

	float4 PosH  : SV_POSITION;
	float4 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;

};


float CalcShadowFactor(float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;

	float depth = shadowPosH.z;

	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);

	float dx = 1.0f / (float)width;

	float percentLit = 0.0f;

	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
	};

	[unroll]

	for (int i = 0; i < 9; ++i)
	{
		percentLit += gShadowMap.SampleCmpLevelZero(gSamShadow, shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit / 9.0f;
}


VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.NormalW = mul(vin.Normal, gRotation);//????????

	float4x4 gWorld = mul(mul(gScale, gRotation), gLocation);

	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	float3 PosW = posW.xyz;//????????


	vout.PosW = PosW;

	vout.PosH = mul(float4(PosW, 1.0f), gViewProj);//MVP????
	vout.TexC = vin.TexC;//UV
	vout.ShadowPosH = mul(posW, gShadowTransform);

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{

	float4 diffuseAlbedo = gNormalMap.Sample(gSamAnisotropicWarp, pin.TexC) * gDiffuseAlbedo;

	pin.NormalW = normalize(pin.NormalW);

	float3 toEyeW = normalize(gEyePosW - pin.PosW);//??????????????

	const float shininess = 1.0f - gRoughness;
	Material mat = { diffuseAlbedo, gFresnelR0, shininess };


	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);


	//????????
	float4 directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);
	//????????
	float4 ambient = gAmbientLight * diffuseAlbedo;

	float4 finalCol = directLight + ambient;

	finalCol.a = gDiffuseAlbedo.a;


	return finalCol;

	//return float4(gShadowMap.Sample(gSamLinearWarp, pin.TexC).rrr, 1.0f);
	//return gNormalMap.Sample(gSamAnisotropicWarp, pin.TexC);
	//return gDiffuseMap.Sample(gSamAnisotropicWarp, pin.TexC);
	//return pow((pin.NormalW + 1) * 0.5,1/2.2f);
}




