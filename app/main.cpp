//
// Created by intwi on 2025/12/30.
//

#include <iostream>
#include <wistlib/audio_engine.h>

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
	using namespace wwist::audio_engine;
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	_ASSERT(SUCCEEDED(hr));

	std::unique_ptr<AudioEngine> engine =
		std::make_unique<AudioEngine>(AudioEngine::AudioStreamMode::RENDER_EXCLUSIVE);
	hr = engine->Initialize();
	_ASSERT(SUCCEEDED(hr));

	hr = engine->Start();
	_ASSERT(SUCCEEDED(hr));

	while (!(GetKeyState(VK_ESCAPE) & 0x80)) {
		engine->Update();
	}

	hr = engine->Stop();
	_ASSERT(SUCCEEDED(hr));

	engine.reset();

	CoUninitialize();
#ifdef POPUP_CONSOLE
	Sleep(500);
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
