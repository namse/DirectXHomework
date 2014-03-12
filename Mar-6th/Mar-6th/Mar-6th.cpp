//-----------------------------------------------------------------------------
// File: Meshes.cpp
//
// Desc: For advanced geometry, most apps will prefer to load pre-authored
//       Meshes from a file. Fortunately, when using Meshes, D3DX does most of
//       the work for this, parsing a geometry file and creating vertx buffers
//       (and index buffers) for us. This tutorial shows how to use a D3DXMESH
//       object, including loading it from a file and rendering it. One thing
//       D3DX does not handle for us is the materials and textures for a mesh,
//       so note that we have to handle those manually.
//
//       Note: one advanced (but nice) feature that we don't show here is that
//       when cloning a mesh we can specify the FVF. So, regardless of how the
//       mesh was authored, we can add/remove normals, add more texture
//       coordinate sets (for multi-texturing), etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <mmsystem.h>

#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9         g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9   g_pd3dDevice = NULL; // Our rendering device

LPD3DXMESH          g_pMesh = NULL; // Our mesh object in sysmem
D3DMATERIAL9*       g_pMeshMaterials = NULL; // Materials for our mesh
LPDIRECT3DTEXTURE9* g_pMeshTextures = NULL; // Textures for our mesh
DWORD               g_dwNumMaterials = 0L;   // Number of mesh materials

LPD3DXMESH			g_pMesh2 = NULL;
D3DMATERIAL9*       g_pMeshMaterials2 = NULL; // Materials for our mesh
LPDIRECT3DTEXTURE9* g_pMeshTextures2 = NULL; // Textures for our mesh
DWORD               g_dwNumMaterials2 = 0L;   // Number of mesh materials



//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
	// Create the D3D object.
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	// Set up the structure used to create the D3DDevice. Since we are now
	// using more complex geometry, we will create a device with a zbuffer.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// Create the D3DDevice
	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}

	// Turn on the zbuffer
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	// Turn on ambient lighting 
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffa0a0a0 );


	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SetupLights()
// Desc: Sets up the Lights and materials for the scene.
//-----------------------------------------------------------------------------
VOID SetupLights()
{
	// Set up a white, directional light, with an oscillating direction.
	// Note that many Lights may be active at a time (but each one slows down
	// the rendering of our scene). However, here we are just using one. Also,
	// we need to set the D3DRS_LIGHTING renderstate to enable lighting
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof( D3DLIGHT9 ) );
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 0.0f;
	light.Diffuse.b = 0.0f;
	vecDir = D3DXVECTOR3( cosf( timeGetTime() / 350.0f ),
		1.0f,
		sinf( timeGetTime() / 350.0f ) );
	D3DXVec3Normalize( ( D3DXVECTOR3* )&light.Direction, &vecDir );
	light.Range = 10000.0f;
	g_pd3dDevice->SetLight( 0, &light );


	D3DXVECTOR3 vecDir2;
	D3DLIGHT9 light2;
	ZeroMemory( &light2, sizeof( D3DLIGHT9 ) );
	light2.Type = D3DLIGHT_DIRECTIONAL;
	light2.Diffuse.r = 0.0f;
	light2.Diffuse.g = 1.0f;
	light2.Diffuse.b = 0.0f;
	vecDir2 = D3DXVECTOR3( cosf( timeGetTime() / 350.0f + D3DX_PI ),
		1.0f,
		sinf( timeGetTime() / 350.0f  + D3DX_PI ) );
	D3DXVec3Normalize( ( D3DXVECTOR3* )&light2.Direction, &vecDir2 );
	light2.Range = 10000.0f;
	g_pd3dDevice->SetLight( 1, &light2 );
		
	g_pd3dDevice->LightEnable( 0, TRUE );
	g_pd3dDevice->LightEnable( 1, TRUE );
	
    //g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xff111111 );

}

//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Load the mesh and build the material and texture arrays
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	LPD3DXBUFFER pD3DXMtrlBuffer;

	// Load the mesh from the specified file
	if( FAILED( D3DXLoadMeshFromX( L"Tiger.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
		&g_pMesh ) ) )
	{
		// If model is not in current folder, try parent folder
		if( FAILED( D3DXLoadMeshFromX( L"..\\Tiger.x", D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
			&g_pMesh ) ) )
		{
			MessageBox( NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK );
			return E_FAIL;
		}
	}
	
	// 메쉬가 버텍스 포멧으로 D3DFVF_NORMAL을 가지고 있는가?
	if ( !(g_pMesh->GetFVF() & D3DFVF_NORMAL) )
	{
		//가지고 있지 않다면 메쉬를 복제하고 D3DFVF_NORMAL을 추가한다.
		ID3DXMesh* pTempMesh = 0;
		g_pMesh->CloneMeshFVF(
			D3DXMESH_MANAGED,
			g_pMesh->GetFVF() | D3DFVF_NORMAL,  //이곳에 추가
			g_pd3dDevice,
			&pTempMesh );

		// 법선을 계산한다.
		D3DXComputeNormals( pTempMesh, 0 );

		g_pMesh->Release(); // 기존메쉬를 제거한다
		g_pMesh = pTempMesh; // 기존메쉬를 법선이 계산된 메쉬로 지정한다.
	}

	
	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = ( D3DXMATERIAL* )pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	if( g_pMeshMaterials == NULL )
		return E_OUTOFMEMORY;
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if( g_pMeshTextures == NULL )
		return E_OUTOFMEMORY;

	for( DWORD i = 0; i < g_dwNumMaterials; i++ )
	{
		// Copy the material
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;
		if( d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
		{
			// Create the texture
			if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&g_pMeshTextures[i] ) ) )
			{
				// If texture is not in current folder, try parent folder
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s( strTexture, MAX_PATH, strPrefix );
				strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );
				// If texture is not in current folder, try parent folder
				if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
					strTexture,
					&g_pMeshTextures[i] ) ) )
				{
					MessageBox( NULL, L"Could not find texture map", L"Meshes.exe", MB_OK );
				}
			}
		}
	}


	// Done with the material buffer
	pD3DXMtrlBuffer->Release();


	//#@$%!#$%!@#%%^@%$#@%!^%@#!$@!#$!@#$

	LPD3DXBUFFER pD3DXMtrlBuffer2;

	// Load the mesh from the specified file
	if( FAILED( D3DXLoadMeshFromX( L"Tiger2.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer2, NULL, &g_dwNumMaterials2,
		&g_pMesh2 ) ) )
	{
		// If model is not in current folder, try parent folder
		if( FAILED( D3DXLoadMeshFromX( L"..\\Tiger2.x", D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer2, NULL, &g_dwNumMaterials2,
			&g_pMesh2 ) ) )
		{
			MessageBox( NULL, L"Could not find tiger2.x", L"Meshes.exe", MB_OK );
			return E_FAIL;
		}
	}
	
	// 메쉬가 버텍스 포멧으로 D3DFVF_NORMAL을 가지고 있는가?
	if ( !(g_pMesh2->GetFVF() & D3DFVF_NORMAL) )
	{
		//가지고 있지 않다면 메쉬를 복제하고 D3DFVF_NORMAL을 추가한다.
		ID3DXMesh* pTempMesh = 0;
		g_pMesh2->CloneMeshFVF(
			D3DXMESH_MANAGED,
			g_pMesh2->GetFVF() | D3DFVF_NORMAL,  //이곳에 추가
			g_pd3dDevice,
			&pTempMesh );

		// 법선을 계산한다.
		D3DXComputeNormals( pTempMesh, 0 );

		g_pMesh2->Release(); // 기존메쉬를 제거한다
		g_pMesh2 = pTempMesh; // 기존메쉬를 법선이 계산된 메쉬로 지정한다.
	}

	
	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer2
	D3DXMATERIAL* d3dxMaterials2 = ( D3DXMATERIAL* )pD3DXMtrlBuffer2->GetBufferPointer();
	g_pMeshMaterials2 = new D3DMATERIAL9[g_dwNumMaterials2];
	if( g_pMeshMaterials2 == NULL )
		return E_OUTOFMEMORY;
	g_pMeshTextures2 = new LPDIRECT3DTEXTURE9[g_dwNumMaterials2];
	if( g_pMeshTextures2 == NULL )
		return E_OUTOFMEMORY;

	for( DWORD i = 0; i < g_dwNumMaterials2; i++ )
	{
		// Copy the material
		g_pMeshMaterials2[i] = d3dxMaterials2[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		g_pMeshMaterials2[i].Ambient = g_pMeshMaterials2[i].Diffuse;

		g_pMeshTextures2[i] = NULL;
		if( d3dxMaterials2[i].pTextureFilename != NULL &&
			lstrlenA( d3dxMaterials2[i].pTextureFilename ) > 0 )
		{
			// Create the texture
			if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
				d3dxMaterials2[i].pTextureFilename,
				&g_pMeshTextures2[i] ) ) )
			{
				// If texture is not in current folder, try parent folder
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s( strTexture, MAX_PATH, strPrefix );
				strcat_s( strTexture, MAX_PATH, d3dxMaterials2[i].pTextureFilename );
				// If texture is not in current folder, try parent folder
				if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
					strTexture,
					&g_pMeshTextures2[i] ) ) )
				{
					MessageBox( NULL, L"Could not find texture map", L"Meshes.exe", MB_OK );
				}
			}
		}
	}


	// Done with the material buffer
	pD3DXMtrlBuffer2->Release();
	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if( g_pMeshMaterials != NULL )
		delete[] g_pMeshMaterials;
	if( g_pMeshMaterials2 != NULL )
		delete[] g_pMeshMaterials2;

	if( g_pMeshTextures )
	{
		for( DWORD i = 0; i < g_dwNumMaterials; i++ )
		{
			if( g_pMeshTextures[i] )
				g_pMeshTextures[i]->Release();
		}
		delete[] g_pMeshTextures;
	}
	if( g_pMeshTextures2 )
	{
		for( DWORD i = 0; i < g_dwNumMaterials2; i++ )
		{
			if( g_pMeshTextures2[i] )
				g_pMeshTextures2[i]->Release();
		}
		delete[] g_pMeshTextures2;
	}
	if( g_pMesh != NULL )
		g_pMesh->Release();
	
	if( g_pMesh2 != NULL )
		g_pMesh2->Release();
	if( g_pd3dDevice != NULL )
		g_pd3dDevice->Release();

	if( g_pD3D != NULL )
		g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
	// Set up world matrix
	D3DXMATRIXA16 matWorld;
	D3DXMatrixRotationY( &matWorld, timeGetTime() / 1000.0f );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	// Set up our view matrix. A view matrix can be defined given an eye point,
	// a point to lookat, and a direction for which way is up. Here, we set the
	// eye five units back along the z-axis and up three units, look at the 
	// origin, and define "up" to be in the y-direction.
	D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
	// Clear the backbuffer and the zbuffer
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 0 );

	// Begin the scene
	if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		// Setup the world, view, and projection matrices
		SetupMatrices();

		SetupLights();

		// Meshes are divided into subsets, one for each material. Render them in
		// a loop
		for( DWORD i = 0; i < g_dwNumMaterials; i++ )
		{
			// Set the material and texture for this subset
			g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
			g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );

			// Draw the mesh subset
			g_pMesh->DrawSubset( i );
		}
		D3DXMATRIXA16 mat;
		D3DXMatrixTranslation( &mat, 1,0,0 );
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &mat );
		for( DWORD i = 0; i < g_dwNumMaterials2; i++ )
		{
			// Set the material and texture for this subset
			g_pd3dDevice->SetMaterial( &g_pMeshMaterials2[i] );
			g_pd3dDevice->SetTexture( 0, g_pMeshTextures2[i] );

			// Draw the mesh subset
			g_pMesh->DrawSubset( i );
		}

		// End the scene
		g_pd3dDevice->EndScene();
	}
	LPDIRECT3DTEXTURE9* pTemp = g_pMeshTextures;
	g_pMeshTextures = g_pMeshTextures2;
	g_pMeshTextures2 = pTemp;
	// Present the backbuffer contents to the display
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	UNREFERENCED_PARAMETER( hInst );

	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx( &wc );

	// Create the application's window
	HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 06: Meshes",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL );

	// Initialize Direct3D
	if( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		// Create the scene geometry
		if( SUCCEEDED( InitGeometry() ) )
		{
			// Show the window
			ShowWindow( hWnd, SW_SHOWDEFAULT );
			UpdateWindow( hWnd );

			// Enter the message loop
			MSG msg;
			ZeroMemory( &msg, sizeof( msg ) );
			while( msg.message != WM_QUIT )
			{
				if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
				else
					Render();
			}
		}
	}

	UnregisterClass( L"D3D Tutorial", wc.hInstance );
	return 0;
}



