
extern "C" {
	#define PROCESS_VM_READ 0x0010
	#define PROCESS_QUERY_INFORMATION 0x0400

	typedef struct HWND__* HWND;
	typedef unsigned int uintptr_t;

	__declspec(dllimport) HWND __stdcall FindWindowA(const char* lpClassName, const char* lpWindowName);
	__declspec(dllimport) unsigned long __stdcall GetWindowThreadProcessId(HWND hWnd, unsigned long* lpdwProcessId);
	__declspec(dllimport) void* __stdcall OpenProcess(unsigned long dwDesiredAccess, int bInheritHandle, unsigned long dwProcessId);
	__declspec(dllimport) int __stdcall ReadProcessMemory(void* hProcess, const void* lpBaseAddress, void* lpBuffer, size_t nSize, size_t* lpNumberOfBytesRead);
	__declspec(dllimport) int __stdcall CloseHandle(void* hObject);
}

class Game {
public:
	void* handle = nullptr;
	unsigned long pid = 0;

	bool Connect(const char* windowName) {
		HWND hwnd = FindWindowA(nullptr, windowName);
		if (!hwnd)
			return false;

		GetWindowThreadProcessId(hwnd, &pid);
		handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, pid);
		return handle != nullptr;
	}

	template <typename T>
	T Read(uintptr_t addr) {
		T buffer;
		ReadProcessMemory(handle, (const char*)addr, &buffer, sizeof(T), nullptr);
		return buffer;
	}
};
