#include "StdAfx.h"
#include "Engine.h"
#include <iostream>
using namespace std;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	try{
		Engine engine(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
		engine.BeginLoop();
	} catch (exception& e){
		MessageBox(NULL, e.what(), "Fatal error!", MB_ICONERROR|MB_DEFAULT_DESKTOP_ONLY);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}