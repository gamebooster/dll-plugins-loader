#include <windows.h>
#include <string>
#include <iostream>
#include <vector>

std::wstring GetExecutablePath() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}

std::vector<std::wstring> GetAllFileNamesFromFolder(std::wstring folder)
{
	std::vector<std::wstring> names;
	wchar_t search_path[200];
	wsprintf(search_path, L"%s*.*", folder.c_str());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

HANDLE thread = nullptr;

DWORD WINAPI InitializeHook(void* arguments) {
	auto exe_path = GetExecutablePath();
	exe_path += L"\\plugins\\";

	auto plugin_names = GetAllFileNamesFromFolder(exe_path);

	for(auto name : plugin_names) {
	  auto full_path = (exe_path + name);
      if (LoadLibrary(full_path.c_str())) {
		OutputDebugString((L"Loaded plugin: " + full_path).c_str());
      }
	}

	return 1;
}

void FinalizeHook() {
	
}

extern "C" UINT_PTR mProcs[12] = { 0 };

LPCSTR import_names[] = { "DirectSoundCaptureCreate", "DirectSoundCaptureCreate8", "DirectSoundCaptureEnumerateA",
                          "DirectSoundCaptureEnumerateW", "DirectSoundCreate", "DirectSoundCreate8", "DirectSoundEnumerateA",
	                      "DirectSoundEnumerateW", "DirectSoundFullDuplexCreate", "DllCanUnloadNow", "DllGetClassObject", "GetDeviceID" };

int WINAPI DllMain(HINSTANCE instance, DWORD reason, PVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		TCHAR expandedPath[MAX_PATH];
		ExpandEnvironmentStrings(L"%WINDIR%\\System32\\dsound.dll", expandedPath, MAX_PATH);
		auto module = LoadLibrary(expandedPath);
		if (!module) return 1;
		for (int i = 0; i < 12; i++)
			mProcs[i] = (UINT_PTR)GetProcAddress(module, import_names[i]);

		thread = CreateThread(nullptr, 0, InitializeHook, 0, 0, nullptr);
	}
	else if (reason == DLL_PROCESS_DETACH) {
		//FinalizeHook();
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
	}
	return 1;
}


extern "C" void DirectSoundCaptureCreate_wrapper();
extern "C" void DirectSoundCaptureCreate8_wrapper();
extern "C" void DirectSoundCaptureEnumerateA_wrapper();
extern "C" void DirectSoundCaptureEnumerateW_wrapper();
extern "C" void DirectSoundCreate_wrapper();
extern "C" void DirectSoundCreate8_wrapper();
extern "C" void DirectSoundEnumerateA_wrapper();
extern "C" void DirectSoundEnumerateW_wrapper();
extern "C" void DirectSoundFullDuplexCreate_wrapper();
extern "C" void DllCanUnloadNow_wrapper();
extern "C" void DllGetClassObject_wrapper();
extern "C" void GetDeviceID_wrapper();