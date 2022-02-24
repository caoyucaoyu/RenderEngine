#include "Head.h"
#include <WindowsX.h>
#include "D3DInit.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,PSTR cmdLine, int showCmd)
{

	D3DInit D3dapp;
	D3dapp.Init(hInstance);
	return D3dapp.Run();
}

