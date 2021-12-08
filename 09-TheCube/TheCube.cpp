//--------------------------------------------------------------------------------------
// File: TheCube.cpp
//
// This application displays a cube in Direct3D 10
// It will be used to demonstrate various DirectX techniques
//
// Follows on from the early Microsoft DX10 tutorials
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <atlbase.h>
#include "resource.h"

#include "CTimer.h" // Timer class - not DirectX


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// The content of a single vertex in the geometry to render. Currently just stores the
// position of the vertex (x,y,z). However, we can store other information for each
// vertex (for example, a colour for the vertex) so we use a structure.
struct SimpleVertex
{
	D3DXVECTOR3 Pos; // A D3DXVECTOR3 holds (x,y,z) coordinates. It can be used to hold either a point or a vector.
					 // Be careful, D3D does not distinguish between points and vectors, but the maths theory does.
					 // In this case it is a vertex in our geometry, so we know it is a point (often abreviated to
					 // 'Pos' for position - the x,y,z coordinates are the position of a point)
	D3DXVECTOR4 Colour; //DG: D3DXVECTOR4 storing the colour of the vertex in RGBA format
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

// The main D3D interface, this pointer is used to access most D3D functions
ID3D10Device* g_pd3dDevice = NULL;

// Variables used to setup D3D
IDXGISwapChain* g_pSwapChain = NULL;
ID3D10Texture2D* g_pDepthStencil = NULL;
ID3D10DepthStencilView* g_pDepthStencilView = NULL;
ID3D10RenderTargetView* g_pRenderTargetView = NULL;

// Variables used for rendering
ID3D10InputLayout* g_pVertexLayout = NULL;
ID3D10Buffer* g_pVertexBuffer = NULL;
ID3D10Effect* g_pEffect = NULL;
ID3D10EffectTechnique* g_pTechnique = NULL;

// Matrix variables
ID3D10EffectMatrixVariable* g_pWorldVariable = NULL;
ID3D10EffectMatrixVariable* g_pViewVariable = NULL;
ID3D10EffectMatrixVariable* g_pProjectionVariable = NULL;
D3DXMATRIX                  g_World;
D3DXMATRIX                  g_View;
D3DXMATRIX                  g_Projection;

// Variables used to setup the Window
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;

// Other
CTimer timer;


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------

// Declare functions in advance so we can use them before their definition
HRESULT InitDevice();
void CleanupDevice();
void Render();
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);



////////////////////////////////////////////////////////////////////////////////////////
// Direct3D Setup and Rendering
////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	// Many DirectX functions return a "HRESULT" variable to indicate success or failure. Microsoft code often uses
	// the FAILED macro to test this variable, you'll see it throughout the code - it's fairly self explanatory.
	HRESULT hr = S_OK;


	////////////////////////////////
	// Initialise Direct3D

	// Calculate the visible area the window we are using - the "client rectangle" refered to in the first function is the 
	// size of the interior of the window, i.e. excluding the frame and title
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;


	// Create a Direct3D device (i.e. initialise D3D), and create a swap-chain (create a back buffer to render to)
	// Key information is the size of the window, the pixel format (R8G8B8A8 - recall the lectures about colour formats),
	// the refresh rate of the monitor, the window we are targeting and whether we want to go full-screen.
	//
	// Other details are more rare to change (such as whether we want anti-aliasing)
	//
	DXGI_SWAP_CHAIN_DESC sd;         // Structure to contain all the information needed
	ZeroMemory(&sd, sizeof(sd)); // Clear the structure to 0 - common Microsoft practice, not really good style
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.OutputWindow = g_hWnd;
	sd.Windowed = TRUE;
	hr = D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0,
		D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice);
	if (FAILED(hr))
		return hr;


	// Specify the render target as the back-buffer - this is an advanced topic. DirectX 10 can interpret memory in different
	// ways. Just as we can cast an "int" to a "char" in C++, DX10 can change the interepretation of a block of memory. For
	// example, it can consider a bitmap (texture) as a viewport if required.
	//
	// This code is getting the back buffer and interpreting it as a "Render Target" - something we can render to. This is
	// the normal interpretation for the back buffer, so this code almost always occurs in the standard D3D setup
	//
	ID3D10Texture2D* pBackBuffer;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;
	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create a texture (bitmap) to use for a depth buffer
	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D10_USAGE_DEFAULT;
	descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;
	// Create the depth stencil view, i.e. indicate that the texture
	// just created is to be used as a depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV,
		&g_pDepthStencilView);
	if (FAILED(hr)) return hr;


	// Select the back buffer and depth buffer to use for rendering now
	g_pd3dDevice->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);


	// Setup the viewport - defines which part of the window we will render to, almost always the whole window
	D3D10_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp);



	////////////////////////////////
	// Initialise Shader Effects

	// Create an effect - this is the vertex and pixel shader programs that will be used for rendering. They are loaded from
	// a file (ending ".fx"). We will not look in detail at shaders yet.
	//
	ID3D10Blob* pErrors;
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	hr = D3DX10CreateEffectFromFile(L"TheCube.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0,
		g_pd3dDevice, NULL, NULL, &g_pEffect, &pErrors, NULL);
	if (FAILED(hr))
	{
		if (pErrors != 0)
		{
			MessageBox(NULL, CA2CT(static_cast<char*>(pErrors->GetBufferPointer())), L"Error", MB_OK);
		}
		else
		{
			MessageBox(NULL, L"Error loading FX file. Ensure you run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		}
		return hr;
	}
	g_pTechnique = g_pEffect->GetTechniqueByName("Render");

	// The matrices we manipulate in this C++ program are passed to the shaders to actually affect the rendering
	// We will see much more of this when we look at shaders in detail
	g_pWorldVariable = g_pEffect->GetVariableByName("World")->AsMatrix();
	g_pViewVariable = g_pEffect->GetVariableByName("View")->AsMatrix();
	g_pProjectionVariable = g_pEffect->GetVariableByName("Projection")->AsMatrix();



	////////////////////////////////
	// Define the geometry

	// Define the input layout. Go to the top of the file and look at the SimpleVertex structure, then return here. Each vertex in the
	// geometry we render will contain the data in the SimpleVertex structure, just the (x,y,z) position at the beginning of this example.
	//
	// We need to tell D3D about the contents of that structure (it is not going to read our C++ code!). The input layout is a special
	// D3D structure to describe the vertex data. There is one row in the layout array below for each data element in our structure.
	// So the row below describes the fact that each vertex in our geometry has a position which is made of three 32-bit floats. This should
	// also match the vertex shader input structure. So, in total there are three structures maintained to keep C++ code and shader working
	// correctly together. Watch out for this detail when working on advanced code in the future.
	//
	// D3D refers to most data as though it was colour data, which is why the constant below is ...R32G32B32... rather than ...X32Y32Z32...
	// (this is another example of D3D reinterpreting data). In the DirectX documentation for this tutorial there is a more detailed 
	// description of the layout data below, you may wish to skim through that now
	//
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		// Semantic   Index  Format                          Slot  Offset  Slot Class                    Instance Step
		{ "POSITION", 0,     DXGI_FORMAT_R32G32B32_FLOAT,    0,    0,      D3D10_INPUT_PER_VERTEX_DATA,  0 },
		{ "COLOR",    0,     DXGI_FORMAT_R32G32B32A32_FLOAT, 0,    12,     D3D10_INPUT_PER_VERTEX_DATA,  0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);


	// Create and set the input layout. The code above just defined the layout array, here it is passed to D3D and set ready for use, i.e. we
	// indicate that the next geometry data that D3D can expect will be in the above form
	//
	D3D10_PASS_DESC PassDesc;
	g_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &g_pVertexLayout);
	if (FAILED(hr))
		return hr;
	g_pd3dDevice->IASetInputLayout(g_pVertexLayout);



	//DG: The vertices of the cube, optimized for triangle strip topology
	SimpleVertex cubeVertices[] =
	{
		//RED
		D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR4(0.5f, 0.0f, 0.0f, 1.0f),
		D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f),
		D3DXVECTOR3(1.0f,-1.0f,-1.0f), D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f),
		D3DXVECTOR3(1.0f, 1.0f,-1.0f), D3DXVECTOR4(1.0f, 0.5f, 0.5f, 1.0f),
		//YELLOW
		D3DXVECTOR3(1.0f,-1.0f, 1.0f), D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f),
		D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR4(1.0f, 1.0f, 0.5f, 1.0f),
		//GREEN
		D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f),
		D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR4(0.5f, 1.0f, 0.5f, 1.0f),
		//CYAN
		D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR4(0.0f, 1.0f, 1.0f, 1.0f),
		D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR4(0.5f, 1.0f, 1.0f, 1.0f),
		//BLUE
		D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR4(0.0f, 0.0f, 0.5f, 1.0f),
		D3DXVECTOR3(1.0f,-1.0f,-1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f),
		D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f),
		D3DXVECTOR3(1.0f,-1.0f, 1.0f), D3DXVECTOR4(0.5f, 0.5f, 1.0f, 1.0f),
		//MAGENTA
		D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR4(0.5f, 0.0f, 0.5f, 1.0f),
		D3DXVECTOR3(1.0f, 1.0f,-1.0f), D3DXVECTOR4(1.0f, 0.0f, 1.0f, 1.0f),
		D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR4(1.0f, 0.0f, 1.0f, 1.0f),
		D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR4(1.0f, 0.5f, 1.0f, 1.0f),
	};
	int numVertices = sizeof(cubeVertices) / sizeof(SimpleVertex);


	// Now create a "vertex buffer", an area of Direct3D controlled memory that will actually hold the vertex data to be rendered. This data is
	// likely to be created in "GPU memory" for performance reasons. The vertex buffer will be initialised with the above array, but once created
	// we cannot change the vertex buffer directly. Key point to understand is the separation between C++ data like the array above and D3D data
	// like this vertex buffer
	//
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER; // Indicate that it is a vertex buffer
	bd.ByteWidth = sizeof(cubeVertices);   // Indicate the size of the vertex buffer
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = cubeVertices;         // The initial data for the vertex buffer - i.e. copy the array above into it
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer - tell D3D that the next geometry data it can expect will be in the vertex buffer we just created
	UINT stride = sizeof(SimpleVertex); // Size of a single vertext
	UINT offset = 0;
	g_pd3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Set primitive topology - tell D3D that the vertex buffer holds a triangle list, i.e. each triplet of vertices should be
	// connected into triangles. There are other topologies, the tutorial documentation discusses this, mentioning triangle strips
	g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	///////////////////////////////////////////////////////////
	// Initialise matrices to position the geometry (& camera)

	// Initialize the world matrix of the triangle. This line sets it to the
	// identity matrix - look up the identity matrix in the lecture notes. Then
	// look at the form of a world matrix - where will the triangle be positioned?
	D3DXMatrixIdentity(&g_World);

	// Initialize the view (camera) matrix - this determines where we view the
	// scene from. We won't cover this here
	D3DXVECTOR3 Eye(0.0f, 0.0f, -5.0f);  // Position of camera
	D3DXVECTOR3 At(0.0f, 0.0f, 0.0f);   // Point the camera is looking at
	D3DXVECTOR3 Up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_View, &Eye, &At, &Up);

	// Initialize the projection matrix - this determines properties of the
	// camera, again we'll see this in more detail later
	D3DXMatrixPerspectiveFovLH(&g_Projection, (float)D3DX_PI * 0.25f, width / (FLOAT)height, 0.1f, 100.0f);

	// Will use a timer class to help in this tutorial (not part of DirectX). It's like a stopwatch - start it counting now
	timer.Start();

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	// The D3D setup and preparation of the geometry created several objects that use up memory (e.g. the vertex buffer)
	// Each object that allocates memory (or hardware resources) needs to be "released" when we exit the program
	// There is similar code in every D3D program, but the list of objects that need to be released depends on what was created
	if (g_pd3dDevice)    g_pd3dDevice->ClearState();

	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pEffect)       g_pEffect->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pSwapChain)    g_pSwapChain->Release();
	if (g_pd3dDevice)    g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
	//////////////////////
	// Update matrices

	// Using a timer class to help here (it's not part of DirectX)
	// It keeps track of the time passed in seconds
	float time = timer.GetTime();

	// Make the triangle's world matrix from a Y-rotation matrix, with the angle = time. So as time progresses
	// the matrix (an so the cube) rotates more and more around the Y-axis. Can combine multiple transformation
	// matrices to make the cubes motion more complex...
	D3DXMATRIX matrixRot;
	D3DXMATRIX matrixScale;
	//DG: rotate along all axes
	D3DXMatrixRotationYawPitchRoll(&matrixRot, time, time, time);
	//DG: continuously increase and decrease along all axes
	D3DXMatrixScaling(&matrixScale, (sin(time) + 2) / 3, (sin(time) + 2) / 3, (sin(time) + 2) / 3);
	g_World = matrixRot * matrixScale;

	// Pass the matrices we've set up in C++ to the shaders (on the GPU). We'll see more about this later
	g_pWorldVariable->SetMatrix((float*)&g_World);
	g_pViewVariable->SetMatrix((float*)&g_View);
	g_pProjectionVariable->SetMatrix((float*)&g_Projection);


	////////////////////////
	// Render the scene

	// Clear the back buffer - before drawing the geometry clear the entire window to a fixed colour
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	g_pd3dDevice->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	g_pd3dDevice->ClearDepthStencilView(g_pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);

	// Render a triangle. We have already told D3D the format of the geometry, and where to find it (in the vertex buffer). We prepared
	// some shaders for rendering. So now there is nothing to do but tell D3D to draw. We will look at the code regarding techniques in more
	// detail later, so for now the only key line is the one containing "Draw"
	D3D10_TECHNIQUE_DESC techDesc;
	g_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		g_pTechnique->GetPassByIndex(p)->Apply(0);
		g_pd3dDevice->Draw(18, 0); //DG: Draw 18 vertices, starting at vertex 0
	}

	// After we've finished drawing to the off-screen back buffer, we "present" it to the front buffer (the screen)
	g_pSwapChain->Present(0, 0);
}



////////////////////////////////////////////////////////////////////////////////////////
// Window Setup
////////////////////////////////////////////////////////////////////////////////////////
// As before

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		// First check to see if there are any messages that need to be processed for the window (window resizing, minimizing, whatever)
		// If not then the window is idle and the D3D rendering occurs. This is in a loop. So the window is rendered over and over, as fast as
		// possible as long as we are not manipulating the window in some way
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 1280, 960 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 10 - Beyond the Tutorials - The Cube", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
