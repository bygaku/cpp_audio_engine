//
// Created by intwi on 2025/12/30.
//

#include <iostream>

#ifdef WIN32
#include <windows.h>

int WINAPI WinMain (HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR	  lpCmdLine,
					int		  nCmdShow) {
#ifdef POPUP_CONSOLE
	AllocConsole();
	freopen_s(reinterpret_cast<FILE **>(stdout), "CONOUT$", "w", stdout);
	freopen_s(reinterpret_cast<FILE **>(stderr), "CONOUT$", "w", stderr);
#endif
	MessageBoxA(nullptr, "[Main]: Hello Project!\n\nSUBSYSTEM:WINDOWS\nBUILD_EXE is true", "Message", MB_OK);

#ifdef POPUP_CONSOLE
	Sleep(1000);
	FreeConsole();
#endif
	return 0;
}

#else
int main(int argv, char* argc[]) {
	std::cout << "[Main]: Hello Project!" << std::endl;
	std::cout << "[Main]: BUILD_EXE is true" << std::endl;
	return 0;
}
#endif
