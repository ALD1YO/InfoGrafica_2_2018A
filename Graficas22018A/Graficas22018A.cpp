// Graficas22018A.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Graficas22018A.h"
#include "CDXManager.h"
#include "CGFX2D.h"
#include "CGFX3D.h"
#include "CMesh.h"
#include "CMeshMathSurface.h"
#include <Math.h>
#include "C3DCamera.h"
#include "CLPFilter.h"

#define MAX_LOADSTRING 100

// Global Variables:
CDXManager g_Manager;
CGFX2D g_GFX2D(&g_Manager);
CGFX3D g_GFX3D(&g_Manager);
CMeshMathSurface g_meshSurface;
CMeshMathSurface g_meshSurface1;
CMesh			g_meshSuzzane;
CMeshMathSurface	g_meshPlane;
C3DCamera		g_Camera;
#define Pi 3.141592f
VECTOR4D Sphere(float u, float v) 
{
	return
	{
		cosf(2 * Pi *u)*cosf(Pi*(v-0.5f)),
		sinf(2 * Pi*u)*cosf(Pi*(v-0.5f)),
		sinf(Pi*(v-0.5f)),1
	};
}
VECTOR4D SphereNormals(const VECTOR4D& Position)
{
	VECTOR4D R = Position;
	R.w = 0;
	return Normalize(R);
}
float Plane(const float x,const float y)
{
	return 0.0f;//plano.z = 0
}
VECTOR4D PlanesNormals(const VECTOR4D& position) 
{
	return { 0,0,1,0 };
}

const float omega = 4.0f; //oh ma gah (omega, magnitud de las ondas)
float SinCos(const float x, const float y) 
{
	return sinf(omega*x)*cos(omega*y);
}
VECTOR4D SinCosNormal(const VECTOR4D& Position)
{
	 /*
		z=f(x,y) -> z-f(x,y) = f(x,y,z) = 0
		N=Nabla*f(x,y,z)) = Dot([d/dx,d/dy,d/dz]*f(x,y,z)	
		z-sin(omega*x)*cos(omega*y)=f(x,y,z)=0
		df(x,y,z)/dx = 0 -omega*cos(omega*x)*cos(omega*y)
		df(x,y,z)/dx = 0 +omega*sin(omega*x)*sin(omega*y)
		df(x,y,z)/dx = 1
		*/
	return Normalize({ -omega * cosf(omega*Position.x)*cosf(omega*Position.y),
		omega*sinf(omega*Position.x)*sinf(omega*Position.y),
		1,0
		});
}
//Aqui cargas las texturas

ID3D11Texture2D* g_pTextureSpecular;
ID3D11ShaderResourceView* g_pSRVSpecular;

ID3D11Texture2D* g_pTextureWater;
ID3D11ShaderResourceView* g_pSRVWater;

ID3D11Texture2D*g_pTextureEmissive;
ID3D11ShaderResourceView* g_pSRVSEmissive;


HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
ID3D11Texture2D* g_pTexture;
ID3D11ShaderResourceView* g_pSRV;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GRAFICAS22018A, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAFICAS22018A));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRAFICAS22018A));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GRAFICAS22018A);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   IDXGIAdapter* pAdapter = g_Manager.EnumAndChooseAdapter(NULL);
   if (!pAdapter) return FALSE;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   if (!g_Manager.Initialize(hWnd, pAdapter, 1280, 720)) 
   {
	   MessageBox(NULL, L"No fue posible iniciar Direct3D", L"Oh no!", MB_ICONERROR);
	   return FALSE;
   }
   if(!g_GFX2D.Initialize())
   {
	   MessageBox(NULL, L"Hubo un error al iniciar el sombreador 2D", 
		   L"Ya valió", MB_ICONERROR);
	   return FALSE;
   }
   if (!g_GFX3D.Initialize())
   {
	   MessageBox(NULL, L"Hubo un error al iniciar el sombreador 3D",
		   L"Ya valió", MB_ICONERROR);
	   return FALSE;
   }
   ///////////
#define RESV 25
   //#tesxt replacement
   //Macros
   //Remplazador no consume memoria

   ///////
   g_meshSurface.BuildAnalyticSurface(
	   RESV, RESV, -1, -1,
	   2.0f / (RESV - 1),
	   2.0f / (RESV - 1),
	   SinCos,
	   SinCosNormal
   );
   g_meshSurface.SetColor(
	   { 1,0,0,1 }, { 0,1,0,1 }, 
	   { 0,0,1,1 }, { 1,1,0,1 });
   g_meshSurface.BuildTextureCoords(1.0f, 1.0f,
	   -1.0f / (RESV - 1), -1.0f / (RESV - 1));
   g_meshSurface1.BuildParametricSurface(RESV, RESV,
	   0, 0,
	   1.0f / (RESV-1),
	   1.0f / (RESV-1),
	   Sphere,
	   SphereNormals
   );
  
   g_meshSurface1.SetColor({ 1,0,0,1 }, { 0,1,0,1 }, { 0,0,1,1 }, { 1,1,0,1 });
   g_meshSurface1.BuildTextureCoords(
	   1.0f, 1.0f, -1.0f / (RESV - 1), -1.0F / (RESV - 1));
   g_pTexture = g_Manager.LoadTexture((char*)"..\\Data\\Earth512x512.bmp", 1, &g_pSRV, 0, 0);
   g_pTextureSpecular = g_Manager.LoadTexture((char*)"..\\Data\\EarthSpecular512x512.bmp",
	   1, &g_pSRVSpecular, 0, 0);

   g_pTextureWater = g_Manager.LoadTexture((char*)"..\\Data\\WaterTexture.bmp",
	   1, &g_pSRVWater, 0, 0);

   g_pTextureEmissive = g_Manager.LoadTexture((char*)"..\\Data\\Tron_tx.bmp",
	   1, &g_pSRVSEmissive, 0, 0);
   
   g_meshSuzzane.LoadSuzzane();
   g_meshPlane.BuildAnalyticSurface(RESV, RESV, -1, -1, 2.0f/ (RESV-1), 2.0f/(RESV-1), Plane, PlanesNormals);
   g_meshPlane.BuildTextureCoords(0, 0,
	   1.0F / (RESV - 1), 1.0f / (RESV - 1));
   g_Camera.InitializeFromViewMatrix(ViewMatrix( { 1, 1, 8, 1 },//pos
	   { 0, 0, 0, 1 },//target
	   { 0, 0, 1, 0 }//up
   ));
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static float t = 0.0f;
	static bool bTurnUp = false;
	static bool bTurnDown = false;
	static bool bTurnRight = false;
	static bool bTurnLeft = false;
	static bool bMoveBack = false;
	static bool bMoveFront = false;
	static bool bMoveRight = false;
	static bool bMoveLeft = false;
	static CLPFilter ftrTurnYAxis(0, 0.1f);
	static CLPFilter ftrTurnXAxis(0, 0.1f);
	static CLPFilter ftrMoveZAxis(0, 0.1f);
	


	//static float s = 1;
	//static float xOffset = 3, yOffset = 3;
    switch (message)
    {

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': bMoveFront = true; break;
		case 'S': bMoveBack = true; break;
		case 'D': bMoveRight = true; break;
		case 'A': bMoveLeft = true; break;
		case VK_UP:	bTurnUp = true; break;
		case VK_DOWN: bTurnDown = true; break;
		case VK_RIGHT: bTurnRight = true; break;
		case VK_LEFT: bTurnLeft = true; break;
		}
		break;

	case WM_KEYUP:
		switch (wParam)
		{
		case 'W': bMoveFront = false; break;
		case 'S': bMoveBack = false; break;
		case 'D': bMoveRight = false; break;
		case 'A': bMoveLeft = false; break;
		case VK_UP: bTurnUp = false; break;
		case VK_DOWN: bTurnDown = false; break;
		case VK_RIGHT: bTurnRight = false; break;
		case VK_LEFT: bTurnLeft = false; break;
		}
		break;

	case WM_SIZE:
		RECT rc;
		GetClientRect(hWnd, &rc);
		g_Manager.Resize(rc.right, rc.bottom);
		break;
	case WM_CREATE:
		SetTimer(hWnd, 1, 10, NULL);
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
		case 1:
			t += 0.01f;
			InvalidateRect(hWnd, 0, false);
			break;
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	/*
	case WM_KEYDOWN:
		switch (wParam) 
		{
		case VK_LEFT:
			xOffset-=1;
			break;
		case VK_RIGHT:
			xOffset+=1;
			break;
		case VK_DOWN:
			yOffset-=1;
			break;
		case VK_UP:
			yOffset+=1;
			break;
		}
		break;
	*/
    case WM_PAINT:
	{
		
		if (bMoveFront) ftrMoveZAxis.Time(0.05f);
		if (bMoveBack)  ftrMoveZAxis.Time(-0.05f);
		if (!bMoveFront && !bMoveBack) ftrMoveZAxis.Time(0.0f);

		if (bTurnLeft) ftrTurnYAxis.Time(-0.01f);
		if (bTurnRight)ftrTurnYAxis.Time(0.01f);
		if (!bTurnLeft && !bTurnRight) ftrTurnYAxis.Time(0);

		if (bTurnUp) ftrTurnXAxis.Time(-0.01f);
		if (bTurnDown) ftrTurnXAxis.Time(0.01f);
		if (!bTurnUp && !bTurnDown) ftrTurnXAxis.Time(0);

		
		//if (bMoveRight) g_Camera.MoveXAxis(0.05f);
		//if (bMoveLeft) g_Camera.MoveXAxis(-0.05f);

		
		
		g_Camera.RotateYAxis(ftrTurnYAxis.m_y);
		g_Camera.RotateXAxis(ftrTurnXAxis.m_y);
		g_Camera.MoveZAxis(ftrMoveZAxis.m_y);
		
		//1.- Crear una vista al render target
		ID3D11Texture2D* pBackBuffer = nullptr;
		g_Manager.GetSwapChain()->GetBuffer(0, IID_ID3D11Texture2D, (void**)&pBackBuffer);
		ID3D11RenderTargetView* pRTV = nullptr;
		g_Manager.GetDevice()->CreateRenderTargetView(pBackBuffer, 0, &pRTV);
		float Color[] = 
		{
			0.1f,0.1f,0.1f,0
		};
		g_Manager.GetContext()->ClearRenderTargetView(pRTV, Color);
		g_Manager.GetContext()->ClearDepthStencilView(g_Manager.GetDepthStencil(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f, 0);
		//2.- Instalar la vista de salida
		g_Manager.GetContext()->OMSetRenderTargets(1, &pRTV, g_Manager.GetDepthStencil());
		//3.- Instalar el view port
		D3D11_TEXTURE2D_DESC dtd;
		pBackBuffer->GetDesc(&dtd);
		D3D11_VIEWPORT vp;
		vp.Height = dtd.Height;
		vp.Width = dtd.Width;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		g_Manager.GetContext()->RSSetViewports(1, &vp);
		g_GFX3D.m_Params.World = RotationZ(t*.5f);
		g_GFX3D.m_Params.View = g_Camera.GetViewMatrix();
		g_GFX3D.m_Params.Projection =//Nunca mezcles una matrix isometrica con una de perspectiva, porque se ve horrible
			//IsometricMatrix(0.5f, 0.5f, 0.1, 10)
			PerspectiveFOVMatrix(Pi/3, 0.1f,50)
			*Scaling(1, (float)dtd.Width / dtd.Height, 1);//Esta matriz de escalacion corrige la relacion de aspecto
		g_GFX3D.m_Params.TexVelocity = { 0.01f ,0.01f, 0.0f, 0.0f };
		g_GFX3D.m_Params.t = t;

		g_GFX3D.m_Params.Options.lx = SHADER_TOON|MAPPING_DIFFUSE;
		g_Manager.GetContext()->PSSetShaderResources(2, 1, &g_pSRVSEmissive);

		//Root es la raiz
		float s = sin(t);
		MATRIX4D Root = Translation(0, 0, 0); //Aqui puedo mover o escalar todo el sistema
		//Root no jala las luces direccionales
		//Configurar iluminación y definir materiales
		CGFX3D::LIGHT L0, L1, L2; //Luz0= Luz puntual, L1=Luz
		memset(&L0, 0, sizeof(L0));
		memset(&L1, 0, sizeof(L1));
		L0.Ambient = { 0,0,0,0};
		L0.Diffuse = { 1.0f,1.0f,0.8f,1 };
		L0.Direction = { cos(3*t),sin(3*t)*cos(t),sin(3*t)*sin(t),0 };//Direccion de la luz.Puede Girar!!!!(sin(t))
		L0.Direction = Normalize(L0.Direction*Root);
		L0.SwitchAndType.lx = LIGHT_ON; //Prender apagar luz
		L0.SwitchAndType.ly = LIGHT_TYPE_DIRECTIONAL;
		L0.Specular = { 1,1,0.8f,1 };

		L1.Ambient = { 0,0,0,0 };
		L1.Attenuation = { 1,0.2f,0,0 };
		L1.Diffuse = { 1,1,1,1 };
		L1.Specular = { 1,1,1,1 };
		float Offset = 2 * sin(2 * t);
		L1.Position = { (5.0f* Offset)*cosf(t),(5.0f+ Offset)*sinf(t), 0,1 };
		L1.Position = L1.Position *Root;
		L1.SwitchAndType.lx = LIGHT_OFF;
		L1.SwitchAndType.ly = LIGHT_TYPE_POINT;

		g_GFX3D.m_Params.Lights[0] = L0;
		g_GFX3D.m_Params.Lights[1] = L1;

		L2.SwitchAndType.lx = LIGHT_OFF;
		L2.SwitchAndType.ly = LIGHT_TYPE_SPOT;
		L2.Position = { 0,0,0,1 };
		L2.Position = L2.Position * Root;
		L2.Diffuse = { 1,1,1,1 };
		L2.Specular = { 1,1,1,1 };
		L2.Ambient = { 0,0,0,0 };
		L2.Direction = { 0,0,-1,0 };
		L2.Direction = Normalize(L2.Direction * Root);//Hay que normalizar primero
		L2.Attenuation = { 0.1f,0.2f,0,0 };
		L2.SpotParams.x = 20.0f;
		g_GFX3D.m_Params.Lights[2] = L2;



		CGFX3D::MATERIAL M;
		M.Ambient = { 1,1,1,1 };
		M.Diffuse = { 0.7f,0.7f,0.7f,1 };
		M.Specular = { 1,1,1,1 };
		M.Emissive = { 0,0,0,0 };
		M.Params.x = 40;	//x = potencia especular
		g_GFX3D.m_Params.Material = M;


		g_GFX3D.UpdateCB();
		g_GFX3D.Configure();
		g_Manager.GetContext()->PSSetShaderResources(0, 1, &g_pSRV);
		g_Manager.GetContext()->PSSetShaderResources(1, 1, &g_pSRVSpecular);
		g_Manager.GetContext()->PSSetSamplers(0, 1, 
			g_Manager.GetSampler(1));
		//Aqui se dibujan las cositas
		//La M de Mario
		//g_GFX3D.Draw(&g_meshSurface);
		////////////PASOS PARA DIBUJAR ALGO//////////////////////////////////
		//El papa siempre va a la derecha como root
		
		MATRIX4D Suzzane =  Scaling(0.5f, 0.5f, 0.5f)
			*RotationX(2*t)
			*RotationY(0.5*t) *RotationZ(3*t) * Root;
		//float c = 1.0f + (0.1f*rand()) / RAND_MAX; //Con esto va a vibrar Suzzi
		g_GFX3D.m_Params.World = Suzzane;
		g_GFX3D.UpdateCB();
		g_GFX3D.Draw(&g_meshSuzzane);

		////////////////////////////////////////////
		//Construir una jerarquia e las superficies planas
		MATRIX4D HalfCube = Scaling(2,2,2)*Translation(0,0,0) * Root;
		g_Manager.GetContext()->PSSetShaderResources(0, 1, &g_pSRVWater); //Con esto se pinta la textura de agua en los cuadrados
		g_GFX3D.m_Params.World = Translation(0, 0, -1)*HalfCube;
		g_GFX3D.UpdateCB();
		g_GFX3D.Draw(&g_meshPlane);

		g_GFX3D.m_Params.World = RotationX(-Pi / 2)*Translation(0, -1, 0)*HalfCube;
		g_GFX3D.UpdateCB();
		g_GFX3D.Draw(&g_meshPlane);

		g_GFX3D.m_Params.World = RotationY(Pi / 2)* Translation(-1, 0, 0)*HalfCube;
		g_GFX3D.UpdateCB();
		g_GFX3D.Draw(&g_meshPlane);

		/*
		CGFX2D::VERTEX2D Triangle[3] =
		{
			//Pos   color	UV's
			{0,1,   1,0,0,	0,0},
			{1,-1,  0,1,0,	1,0},
			{-1,-1, 0,0,1,	1,1},
		};
		g_GFX2D.m_Params.Brightness = { 0.5,0.5,0.5,1 };

		MATRIX4D World = Scaling(s*cos(t), s, s) * RotationZ(3.141592 / 4*t)* RotationX(3.141592 / 4*t) * Translation(xOffset, yOffset, 0);
		g_GFX2D.m_Params.World = Transpose(World);
		g_Manager.GetContext()->PSSetShaderResources(0, 1, &g_pSRV);
		g_GFX2D.Draw(Triangle, 3);
		*/
		g_Manager.GetSwapChain()->Present(1, 0);
		pBackBuffer->Release();
		pRTV->Release();
		ValidateRect(hWnd, NULL);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
