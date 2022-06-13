struct Light
{
    float3 strength;    //��Դ��ɫ������ͨ�ã�
    float falloffStart;     //���ƺ;۹�ƵĿ�ʼ˥������
    float3 direction;   //�����;۹�Ƶķ�������
    float falloffEnd;    //���;۹�Ƶ�˥����������
    float3 position;    //���;۹�Ƶ�����
    float spotPower;    //�۹�������е��ݲ���
};

struct Material
{
    float4 diffuseAlbedo; //���ʷ�����
    float3 fresnelR0; //RF(0)ֵ�������ʵķ�������
    float roughness; //���ʵĴֲڶ�
};


float CalcAttenuation(float d, float falloffEnd, float falloffStart)
{
    //d����ƹ�ľ���
    float att = saturate((falloffEnd - d) / (falloffEnd - falloffStart));
    return att;
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVector)
{
    //����ʯ��˷��̼����������߱�����İٷֱ�
    float3 reflectPercent = R0 + (1.0f - R0) * pow(1 - saturate(dot(normal, lightVector)), 5.0f);
    return reflectPercent;
}



float3 BlinnPhong(Material mat, float3 normal, float3 toEye,
    float3 lightVec, float3 lightStrength)
{
    float m = (1.0f - mat.roughness) * 256.0f; //�ֲڶ��������mֵ
    float3 halfVec = normalize(lightVec + toEye); //�������
    float roughnessFactor = (m + 8.0f) / 8.0f * pow(max(dot(normal, halfVec), 0.0f), m);//�ֲڶ�����
    float3 fresnelFactor = SchlickFresnel(mat.fresnelR0, halfVec, lightVec); //���������

    float3 specAlbedo = fresnelFactor * roughnessFactor;//���淴�䷴����=���������*�ֲڶ�����
    specAlbedo = specAlbedo / (specAlbedo + 1); //�����淴�䷴�������ŵ�[0��1]

    float3 diff_Spec = lightStrength * (mat.diffuseAlbedo.rgb + specAlbedo); //������+�߹ⷴ��=�������*�ܵķ�����
    return diff_Spec;//����������+�߹ⷴ��
}

float3 ComputerDirectionalLight(Light light, Material mat, float3 normal, float3 toEye)
{
    float3 lightVec = -light.direction; //�������͹�Դָ�򶥵�������෴
    float3 lightStrength = max(dot(normal, lightVec), 0.0f) * light.strength; //����ⵥλ����ϵķ��ն�

    //ƽ�й��������+�߹ⷴ��
    return BlinnPhong(mat, normal, toEye, lightVec, lightStrength);
}



float3 ComputerPointLight(Light light, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = light.position - pos; //����ָ����Դ�Ĺ�����
    float distance = length(lightVec);//����͹�Դ�ľ��루����ģ����

    if (distance > light.falloffEnd)
        return 0;   //�������˥������ֱ�ӷ���0��ʡ�ļ�����

    lightVec /= distance;//��һ���ɵ�λ����
    float nDotl = max(dot(lightVec, normal), 0);//�������С��0
    float3 lightStrength = nDotl * light.strength;//����ٵ�λ����ϵķ��նȣ�û����˥����

    //����˥������
    float att = CalcAttenuation(distance, light.falloffEnd, light.falloffStart);
    lightStrength *= att;//˥����ĵ�λ������ն�

    //�������������͸߹ⷴ��
    return BlinnPhong(mat, normal, toEye, lightVec, lightStrength);
}



float3 ComputerSpotLight(Light light, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = light.position - pos; //����ָ��۹�ƹ�Դ�Ĺ�����
    float distance = length(lightVec); //����͹�Դ�ľ��루����ģ����

    if (distance > light.falloffEnd)
        return 0; //�������˥������ֱ�ӷ���0��ʡ�ļ�����

    lightVec /= distance; //��һ���ɵ�λ����
    float nDotl = max(dot(lightVec, normal), 0); //�������С��0
    float3 lightStrength = nDotl * light.strength; //����ٵ�λ����ϵķ��նȣ�û����˥����

    //����˥������
    float att = CalcAttenuation(distance, light.falloffEnd, light.falloffStart);
    lightStrength *= att; //˥����ĵ�λ������ն�

    //����۹��˥������
    float spotFactor = pow(max(dot(-lightVec, light.direction), 0), light.spotPower);
    lightStrength *= spotFactor;

    //����۹�Ƶ�������͸߹ⷴ��
    return BlinnPhong(mat, normal, toEye, lightVec, lightStrength);
}




#define MAX_LIGHTS 16

float4 ComputerLighting(Light lights[MAX_LIGHTS], Material mat, float3 pos, float3 normal, float3 toEye, float3 shadowFactor)
{
    float3 result = 0.0f;
    int i = 0;


#if(NUM_DIR_LIGHTS > 0)
    for (i = 0; i < NUM_DIR_LIGHTS; i++)
    {
        //���ƽ�й�Ĺ��յ��ӣ�����Ӱ��
        result += shadowFactor[i] * ComputerDirectionalLight(lights[i], mat, normal, toEye);
    }
#endif

#if(NUM_POINT_LIGHTS > 0)
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i++)
    {
        //������Դ�Ĺ��յ��ӣ����û����Ӱ��
        result += ComputerPointLight(lights[i], mat, pos, normal, toEye);
    }
#endif

#if(NUM_SPOT_LIGHTS > 0)
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; i++)
    {
        //����۹�ƹ�Դ�Ĺ��յ��ӣ��۹��û����Ӱ��
        result += ComputerSpotLight(lights[i], mat, pos, normal, toEye);
    }
#endif

    return float4(result, 0.0f);
}