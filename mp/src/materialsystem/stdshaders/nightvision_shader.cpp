#include "BaseVSShader.h"
 
#include "post_nightvision_ps20.inc"
#include "passthrough_vs20.inc" //files generated by compiling the fxc files
 
BEGIN_VS_SHADER( post_nightvision, "Help for post_nightVision" ) //Begin the shader
 
	BEGIN_SHADER_PARAMS
	SHADER_PARAM(NVLEVEL, SHADER_PARAM_TYPE_FLOAT, "1.0", "")
	//This is our shader parameter taken from the material file called NVLEVEL, write "$nvlevel" 10 for example in the .vmt
	END_SHADER_PARAMS
 
	SHADER_INIT_PARAMS() //called after parameters have been initialized
	{
	}
 
	SHADER_FALLBACK //doesn't fallback to anything (I know this works on dx9, hasn't been tested on others)
	{
		return 0;
	}
 
	bool NeedsFullFrameBufferTexture( IMaterialVar **params ) const //does this need the full screen? in our case yes
	{
		return true;
	}
 
	SHADER_INIT //initializes the shader
	{
		LoadTexture(BASETEXTURE);
	}
 
	SHADER_DRAW
	{
		SHADOW_STATE
			{
//				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); //enables SHADER_TEXTURE_STAGE0
// 
//				pShaderShadow->EnableDepthWrites( false ); //depth writes aren't needed 
//				int fmt = VERTEX_POSITION;
//				pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 ); //sets the vertex format for the .fxc
//				pShaderShadow->SetVertexShader( "passthrough_vs20", 0 ); //set the vertex shader
//				pShaderShadow->SetPixelShader( "post_nightvision_ps20" ); //set the pixel shader
//				DefaultFog();
				pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
				int fmt = VERTEX_POSITION;
				pShaderShadow->VertexShaderVertexFormat(fmt, 1, 0, 0);

				DECLARE_STATIC_VERTEX_SHADER( passthrough_vs20 );
				SET_STATIC_VERTEX_SHADER( passthrough_vs20 );

				DECLARE_STATIC_PIXEL_SHADER( post_nightvision_ps20 );
				SET_STATIC_PIXEL_SHADER( post_nightvision_ps20 );
				DefaultFog();
			}
		DYNAMIC_STATE
			{
				float Scale = params[NVLEVEL]->GetFloatValue(); // get the value of NVLEVEL and turn it to a float
				float vScale[4] = {Scale, 1, 1, 1}; //new float using NVLEVEL values
				pShaderAPI->SetPixelShaderConstant(0, vScale); //set the first shader variable to our float
				//pShaderAPI->BindStandardTexture( (Sampler_t)SHADER_TEXTURE_STAGE0, (StandardTextureId_t)0 ); //set the shader texture to our frame buffer
				BindTexture(SHADER_SAMPLER0, BASETEXTURE, -1);

				DECLARE_DYNAMIC_VERTEX_SHADER( passthrough_vs20 );
				SET_DYNAMIC_VERTEX_SHADER( passthrough_vs20 );

				DECLARE_DYNAMIC_PIXEL_SHADER( post_nightvision_ps20 );
				SET_DYNAMIC_PIXEL_SHADER( post_nightvision_ps20 );
			}

		Draw(); //draw the shader
	}
 
END_SHADER
