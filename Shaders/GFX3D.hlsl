struct MATERIAL
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Emissive;
	float4 Params;	//x:Specular Power
};
#define LIGHT_OFF 0
#define LIGHT_ON 1
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct LIGHT
{
	//En HLSL los enteros son de 32 bits
	uint4 SwitchAndType; //x: On/Off, y: 0:Directional, 1: Point  2:Spot 
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Position;	//Position Light, Spot Light Only
	float4 Direction;	//Directional Light / Spot Lighting Axis
	float4 Attenuation;	//[k,kl,kq,0] Directional/Spot
	float4 SpotParams;	//Parámetros de Luz de Spot  x=spot param ...
};


struct VERTEX_INPUT
{
	float4 Position:POSITION;
	float4 Normal:NORMAL;
	float4 Color:COLOR;
	float4 TexCoord:TEXCOORD;
};
struct PIXEL_INPUT
{
	float4 Position:SV_Position;	//Posicion en Espacio de Proyeccion
	float4 Q : POSITION;   //Posicion en Espacio de Vista
	float4 Normal:NORMAL;	//Normal ya esta en Espacio de Vista
	float4 Color:COLOR;
	float4 TexCoord:TEXCOORD;
};

#define LIGHTING_AMBIENT  0x00000001
#define LIGHTING_DIFFUSE  0x00000002
#define LIGHTING_SPECULAR 0x00000004
#define LIGHTING_EMISSIVE 0x00000008
#define MAPPING_DIFFUSE	  0x00000010
#define MAPPING_SPECULAR  0x00000020
#define MAPPING_EMISSIVE  0x00000040
#define SHADER_TOON		  0x00000080

cbuffer PARAMS 
{	//Son 16 bytes
	uint4	Options;
	matrix World;
	matrix View;
	matrix Projection;
	matrix WVP; //World -view Projection se agrega para que venga precomputada
	matrix WV;
	MATERIAL Material;
	LIGHT Lights[8];
	float4 TexVelocity;
	float t; //12 byte padding 
			// la t es de 4 bytes
	// Tiempo
};
PIXEL_INPUT VSMain(VERTEX_INPUT Input)  //VERTEX SHADER
{
	PIXEL_INPUT Output;
	Output.Position = mul(Input.Position, WVP);//WVP World -view Projection
	Output.Normal = normalize(mul(Input.Normal, WV));
	Output.Q = mul(Input.Position, WV);
	Output.Color = Input.Color;
	Output.TexCoord = Input.TexCoord;
	return Output;
}
//El orden en el que declaras las texturas en hlsl es muy importante
//por los zocalos??
//Aqui se declara la textura
Texture2D<float4> t2Diffuse; //t0
Texture2D<float4> t2dSpecular; //t1
Texture2D<float4> t2dEmissive;
SamplerState Sampler;

float4 PSMain(PIXEL_INPUT Input):SV_Target //PIXEL SHADER
{	
	float4 Ambient = 0;
	float4 Diffuse = 0;
	float4 Specular = 0;
	float4 V = normalize(float4(0, 0, 0, 1) - Input.Q);
	float4 N = normalize(Input.Normal);

	for (int i = 0; i < 8; i++)
	{
		//Divergencia: En progra tiene que ver algo con los hilos


		if (LIGHT_ON == Lights[i].SwitchAndType.x)
		{
			Ambient += Material.Ambient*Lights[i].Ambient;
			float4 H;

			switch (Lights[i].SwitchAndType.y)
			{
			case LIGHT_TYPE_DIRECTIONAL: 
				Diffuse += Material.Diffuse*Lights[i].Diffuse*
					max(0,-dot(Input.Normal, Lights[i].Direction));
				H = normalize(V - Lights[i].Direction);
				Specular += Material.Specular*
					Lights[i].Specular*
					pow(max(0, dot(H, N)), Material.Params.x);
				break;
			case LIGHT_TYPE_POINT: 
				{
				float4 L, Dist;
				Dist = Input.Q - Lights[i].Position;
				L = normalize(Dist);
				float d = sqrt(dot(Dist, Dist));
				float PointAttenuation = 1.0f/
					dot(float4(1,d,d*d,0), Lights[i].Attenuation);
				H = normalize(V - L);
				Diffuse += Material.Diffuse*Lights[i].Diffuse*
					max(0, -dot(N, L))*PointAttenuation;
				Specular += Material.Specular*Lights[i].Specular*
					pow(max(0, dot(H, N)), Material.Params.x)*PointAttenuation;
				}
				break;
			case LIGHT_TYPE_SPOT: 
			{
				float4 L, Dist;
				Dist = Input.Q - Lights[i].Position;
				L = normalize(Dist);
				float d = sqrt(dot(Dist, Dist));
				float PointAttenuation = 1.0f /
					dot(float4(1, d, d*d, 0), Lights[i].Attenuation);
				H = normalize(V - L);
				float SpotAttenuation =
					pow(max(0, dot(L, Lights[i].Direction)), Lights[i].SpotParams.x);
				Diffuse += Material.Diffuse*Lights[i].Diffuse*
					max(0, -dot(N, L))*PointAttenuation*SpotAttenuation;
				Specular += Material.Specular*Lights[i].Specular*
					pow(max(0, dot(H, N)), Material.Params.x)*PointAttenuation*
					SpotAttenuation;
			}
				break;

			}
		}
	}
	float EmissiveEnabled = 1.0f;
	float4 ColorTexDiffuse = 1.0f;
	float4 ColorTexSpecular = 1.0f;
	float4 ColorTexEmissive = 0.0f;
	float4 PenAttenuation = 1.0f;
	if (!(LIGHTING_AMBIENT & Options.x)) Ambient = 0;
	if (!(LIGHTING_DIFFUSE & Options.x)) Diffuse = 1;
	if (!(LIGHTING_SPECULAR & Options.x)) Specular = 0;
	if (!(LIGHTING_EMISSIVE & Options.x)) EmissiveEnabled = 0;
	if (MAPPING_DIFFUSE & Options.x)
		ColorTexDiffuse = t2Diffuse.Sample(Sampler, Input.TexCoord.xy);
	if (MAPPING_SPECULAR & Options.x)
		ColorTexSpecular = t2dSpecular.Sample(Sampler, Input.TexCoord.xy);
	if(MAPPING_EMISSIVE & Options.x)
		ColorTexEmissive = t2dEmissive.Sample(Sampler, Input.TexCoord.xy);
	if (SHADER_TOON & Options.x)
	{
		float costheta = abs(dot(N, float4(0, 0, 1, 0)));
		if (costheta < 0.7) PenAttenuation = 0.5f;
		if (costheta < 0.3) PenAttenuation = 0.0f;
	}
	return (Ambient +
		Diffuse* ColorTexDiffuse+
		Specular* ColorTexSpecular + 
		Material.Emissive* EmissiveEnabled + ColorTexEmissive)* PenAttenuation;
}


/*

//Luz tipo silent Hill solo direccion
//float4 DirLuz = float4(.7071,.7071,1,0);//Horizontal, vertical, en z Screenspace
//ultimo parametro siempre es 0 porque es un vector :v

float4 DirLuz = float4(cos(t), sin(t), 1, 0); //Con los cos y sin se mueve la luz

float4 ColorLuz = float4 (1, 1, 1, 1); //RGB mas alfa

float4 DirLuz1 = float4 (-1, 0, 0, 0);
float4 ColorLuz1 = float4 (1, 0, 0, 1);



//float I = -dot(Input.Normal, DirLuz); //I = Intensidad de la luz

float I = max(0,-dot(Input.Normal, DirLuz));
float I1 = max(0, -dot(Input.Normal, DirLuz1));

//return (ColorLuz * I + float4(0.1,0.1,0.2,0))* Color de la luz para regresarlo
//El vector es la luz ambiental

return (ColorLuz1*I1 + ColorLuz * I + float4(0.1, 0.1, 0.2, 0))*
Diffuse.Sample(Sampler, Input.TexCoord.xy);

//return Input.Color*Diffuse.Sample(Sampler, Input. TexCoord.xy
//+TexVelocity.xy*t);

//Sin color
//return Diffuse.Sample(Sampler, Input.TexCoord.xy
//+ TexVelocity.xy*t);


*/