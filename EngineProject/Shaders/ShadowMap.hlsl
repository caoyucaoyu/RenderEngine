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
	float3 PosW  : POSITION;
	float4 PosH  : SV_POSITION;
	float4 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.NormalW = mul(vin.Normal, gRotation);//法线变换

	float4x4 gWorld = mul(mul(gScale, gRotation), gLocation);

	float3 PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;//世界变换

	vout.PosW = PosW;

	vout.PosH = mul(float4(PosW, 1.0f), gViewProj);//MVP矩阵
	vout.TexC = vin.TexC;//UV

	return vout;
}

void PS(VertexOut pin)
{

}


//float4 PS(VertexOut pin) : SV_Target
//{
//	float4 diffuseAlbedo = gNormalMap.Sample(gSamAnisotropicWarp, pin.TexC) * gDiffuseAlbedo;
//	
//	pin.NormalW = normalize(pin.NormalW);
//	
//	float3 toEyeW = normalize(gEyePosW - pin.PosW);//点指向相机向量
//	
//	const float shininess = 1.0f - gRoughness;
//	Material mat = { diffuseAlbedo, gFresnelR0, shininess };
//	float3 shadowFactor = 1.0f;//阴影系数
//	
//	//直接光照
//	float4 directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);
//	//环境光照
//	float4 ambient = gAmbientLight * diffuseAlbedo;
//	
//	float4 finalCol = directLight + ambient;
//	
//	finalCol.a = gDiffuseAlbedo.a;
//	
//	return finalCol;
//}

