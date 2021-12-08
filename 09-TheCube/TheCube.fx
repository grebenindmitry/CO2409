//--------------------------------------------------------------------------------------
// File: TheCube.fx
//
// Follows on from the early Microsoft DX10 tutorials
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

/****| INFO |***************************************************************************/
// The structure below describes the vertex data to be sent into the vertex shader.
// The vertex shader is early in the Direct3D pipeline - it takes our 3D geometry, one 
// vertex at a time, and transforms it into 2D ready for rendering on the viewport.
// This structure should match the vertices typed into the C++ program (in particular,
// it should match the structure at the top of the "TheCube.cpp")
//
// However, this shader is written in HLSL, not C++. The types and syntax are similar,
// but don't exactly match between the languages, but you should be able to work it out.
// If you compare this structure with the one in the C++, you will see that there is
// extra data here. The vertex shader is expecting us to provide it with more data from
// the cpp file. Your exercise is to update the cpp file to provide that extra data. You
// will need to update the vertex structure and the cube vertex data in the cpp file. No
// changes are required in this file...
struct VS_INPUT
{
    float3 Pos    : POSITION;
    float4 Colour : COLOR;
};


// The vertex shader processes our input geometry and outputs data to be used by the rest
// of the pipeline. Here it outputs a 2D position (we'll see later why it's a float4) and
// it passes on the colour (which it didn't use - it's just sending it on to the pixel shader)
struct VS_OUTPUT
{
    float4 ProjPos : SV_POSITION;
    float4 Colour  : COLOR0;
};


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

// The matrices created & manipulated in the C++ code are passed into the shader here
matrix World;
matrix View;
matrix Projection;


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

// The vertex shader will process each of the vertices in the cube (the array of data
// created in the C++ program) and output processed data to the remainder of the pipeline
//
VS_OUTPUT SimpleVertexShader( VS_INPUT vIn )
{
	VS_OUTPUT vOut;
	
	// Use matrices to transform the geometry to 2D (will cover this later)
	float4 WorldPos = mul( float4(vIn.Pos, 1.0f),  World );
	float4 ViewPos  = mul( WorldPos, View );
	vOut.ProjPos    = mul( ViewPos,  Projection );
	
	// Pass the colour unchanged on to the rest of the pipeline (the pixel shader will use it)
	vOut.Colour = vIn.Colour;
	return vOut;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

// The pixel shader renders each pixel. The earlier non-programable rasterizer stage has
// already selected each pixel in each 2D triangle to render. It also blended the colours
// from the three vertices in the triangle to give a colour for this pixel. However, that
// process is fairly invisible to us - the pixel shader just gets the output of the vertex
// shader as it's input. The only job of the pixel shader is to return the final colour
// for the pixel, which is simple in this case
//
float4 SimplePixelShader( VS_OUTPUT vOut ) : SV_Target
{
    return vOut.Colour; // Draw each pixel with colour passed from the vertex shader
}


//--------------------------------------------------------------------------------------

RasterizerState CullNone
{
	CullMode = None;
};

technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, SimpleVertexShader() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, SimplePixelShader() ) );
        
        SetRasterizerState( CullNone ); 
    }
}
