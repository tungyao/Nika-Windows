#ifndef KDLIBMODE

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>

#include "./mapper/kdmapper.hpp"
#include "drvrecode_eac.hpp"
#include "mapper/utils.hpp"
#include "vals.hpp"

HANDLE iqvw64e_device_handle;


LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
		Log(L"[!!] Crash at addr 0x" << ExceptionInfo->ExceptionRecord->ExceptionAddress << L" by 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << std::endl);
	else
		Log(L"[!!] Crash" << std::endl);

	if (iqvw64e_device_handle)
		intel_driver::Unload(iqvw64e_device_handle);

	return EXCEPTION_EXECUTE_HANDLER;
}

int paramExists(const int argc, wchar_t** argv, const wchar_t* param) {
	size_t plen = wcslen(param);
	for (int i = 1; i < argc; i++) {
		if (wcslen(argv[i]) == plen + 1ull && _wcsicmp(&argv[i][1], param) == 0 && argv[i][0] == '/') { // with slash
			return i;
		}
		else if (wcslen(argv[i]) == plen + 2ull && _wcsicmp(&argv[i][2], param) == 0 && argv[i][0] == '-' && argv[i][1] == '-') { // with double dash
			return i;
		}
	}
	return -1;
}

void help() {
	Log(L"\r\n\r\n[!] Incorrect Usage!" << std::endl);
	Log(L"[+] Usage: kdmapper.exe [--free][--mdl][--PassAllocationPtr] driver" << std::endl);
}

bool callbackExample(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize, ULONG64 mdlptr) {
	UNREFERENCED_PARAMETER(param1);
	UNREFERENCED_PARAMETER(param2);
	UNREFERENCED_PARAMETER(allocationPtr);
	UNREFERENCED_PARAMETER(allocationSize);
	UNREFERENCED_PARAMETER(mdlptr);
	Log("[+] Callback example called" << std::endl);

	/*
	This callback occurs before call driver entry and
	can be usefull to pass more customized params in
	the last step of the mapping procedure since you
	know now the mapping address and other things
	*/
	return true;
}

DWORD getParentProcess()
{
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	DWORD ppid = 0, pid = GetCurrentProcessId();

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	__try {
		if (hSnapshot == INVALID_HANDLE_VALUE) __leave;

		ZeroMemory(&pe32, sizeof(pe32));
		pe32.dwSize = sizeof(pe32);
		if (!Process32First(hSnapshot, &pe32)) __leave;

		do {
			if (pe32.th32ProcessID == pid) {
				ppid = pe32.th32ParentProcessID;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));

	}
	__finally {
		if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
	}
	return ppid;
}

//Help people that don't understand how to open a console
void PauseIfParentIsExplorer() {
	DWORD explorerPid = 0;
	GetWindowThreadProcessId(GetShellWindow(), &explorerPid);
	DWORD parentPid = getParentProcess();
	if (parentPid == explorerPid) {
		Log(L"[+] Pausing to allow for debugging" << std::endl);
		Log(L"[+] Press enter to close" << std::endl);
		std::cin.get();
	}
}

bool find_driver() {
	auto driver_handle = CreateFileW(DRIVERNAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
		return false;

	CloseHandle(driver_handle);
	return true;
}

int Inject() {
	SetUnhandledExceptionFilter(SimplestCrashHandler);

	bool free = 0;
	bool mdlMode = 0;
	bool indPagesMode = 0;
	bool passAllocationPtr = 0;

	if (free) {
		Log(L"[+] Free pool memory after usage enabled" << std::endl);
	}

	if (mdlMode) {
		Log(L"[+] Mdl memory usage enabled" << std::endl);
	}

	if (indPagesMode) {
		Log(L"[+] Allocate Independent Pages mode enabled" << std::endl);
	}

	if (passAllocationPtr) {
		Log(L"[+] Pass Allocation Ptr as first param enabled" << std::endl);
	}

	if (find_driver())
	{
		Log(L"[-] Driver loaded" << std::endl);
		return 0;
	}

	iqvw64e_device_handle = intel_driver::Load();

	if (iqvw64e_device_handle == INVALID_HANDLE_VALUE) {
		PauseIfParentIsExplorer();
		return -1;
	}




	NTSTATUS exitCode = 0;
	if (!kdmapper::MapDriver(iqvw64e_device_handle, drvrecode_eac, 0, 0, free, true, 0, passAllocationPtr, callbackExample, &exitCode)) {
		Log(L"[-] Failed to map ApexFreeCheat driver" << std::endl);
		intel_driver::Unload(iqvw64e_device_handle);
		PauseIfParentIsExplorer();
		return -1;
	}

	if (!intel_driver::Unload(iqvw64e_device_handle)) {
		Log(L"[-] Warning failed to fully unload vulnerable driver " << std::endl);
		PauseIfParentIsExplorer();
	}
	Log(L"[+] success" << std::endl);
}



int UnloadInject() {
	//if (!Driver::initialize() || !CheckDriverStatus()) {
	//	UNICODE_STRING VariableName = RTL_CONSTANT_STRING(VARIABLE_NAME);
	//	NtSetSystemEnvironmentValueEx(
	//		&VariableName,
	//		&DummyGuid,
	//		0,
	//		0,
	//		ATTRIBUTES);//delete var

	//	std::cout << "No EFI Driver found\n";
	//	system("pause");
	//	exit(1);
	//	return 1;
	//}
}
#endif