cbuffer PARAMS//Parametros dude, valores iniciales por defecto 0
{
	matrix World; //4x4 float 
	float4 Brightness; //Color
	float Time;	//Escalar del tiempo
};
struct VERTEX2D
{	//Tipo nombre     semantica
	float2 position : POSITION;
	float3 color : COLOR;
	float2 texCoord : TEXCOORD;
};

struct VERTEX2D_OUTPUT 
{
	float4 position : SV_POSITION;
	float4 color:COLOR;
	float2 texCoord : TEXCOORD;
};

VERTEX2D_OUTPUT VSMain(VERTEX2D Input)
{
	VERTEX2D_OUTPUT Output;
	float4 Position = float4(Input.position, 0, 1);

	Output.position = mul(Position, World);//mul = combinacion lineal(suma de productos)
	Output.color = float4(Input.color, 1);
	Output.texCoord = Input.texCoord;
	return Output;
}
//Tipodato formato(rgba)
Texture2D<float4> Diffuse;//Shader Resourse View (Texture2D)
SamplerState Sampler;
float4 PSMain(VERTEX2D_OUTPUT Input) :SV_Target
{

	Input.color = sin(mul(mul(Input.color, Brightness),Diffuse.Sample(Sampler,Input.texCoord)));

return Input.color;
}
//Por si quieres regresar mas datos en un pixel shader
//struct PSOUTPUT
//{
//	float4 Color:SV_Target;
//}
//SV_Target osea que el shader regresara datos para el "lienzo" o target renderer