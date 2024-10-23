#pragma once
#include <TlHelp32.h>
#include <cstdint>
#include <basetsd.h>
#include <WTypesbase.h>


uintptr_t virtualaddy;

#define dtbfix_code CTL_CODE(FILE_DEVICE_UNKNOWN, 0x101, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define rw_code CTL_CODE(FILE_DEVICE_UNKNOWN, 0x102, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define base_code CTL_CODE(FILE_DEVICE_UNKNOWN, 0x103, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _rw {
	INT32 process_id;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
	BOOLEAN write;
} rw, * prw;

typedef struct _ba {
	INT32 process_id;
	ULONGLONG* address;
} ba, * pba;
typedef struct _dtb {
	INT32 process_id;
	bool* operation;
} dtb, * dtbl;
typedef struct _ga {
	ULONGLONG* address;
} ga, * pga;
bool CheckDriverStatus() {

	return true;
}

namespace mem {
	inline HANDLE driver_handle;
	inline INT32 process_id;
	inline bool isMEM = false;
	inline bool init_mem() {
	
	
	}
	inline bool find_driver2() {
	
		isMEM = 1;
		return 1;
	}
	inline bool find_driver() {
		driver_handle = CreateFileW(L"\\\\.\\Windowsloader", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
			return false;

		return true;
	}

	inline void read_physical(PVOID address, PVOID buffer, DWORD size) {
	
		_rw arguments = { 0 };

		//arguments.security = code_security;
		arguments.address = (ULONGLONG)address;
		arguments.buffer = (ULONGLONG)buffer;
		arguments.size = size;
		arguments.process_id = process_id;
		arguments.write = FALSE;

		DeviceIoControl(driver_handle, rw_code, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
	}

	inline void write_physical(PVOID address, PVOID buffer, DWORD size) {
	
		_rw arguments = { 0 };

		arguments.address = (ULONGLONG)address;
		arguments.buffer = (ULONGLONG)buffer;
		arguments.size = size;
		arguments.process_id = process_id;
		arguments.write = TRUE;

		DeviceIoControl(driver_handle, rw_code, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
	}

	inline uintptr_t find_image() {

		uint64_t image_address = { NULL };

		//image_address = Driver::GetBaseAddress(process_id);

		_ba arguments = { NULL };

		arguments.process_id = process_id;
		arguments.address = &image_address;

		DeviceIoControl(driver_handle, base_code, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);

		return image_address;
	}
	inline bool CR3() {
	
		ULONGLONG ret = false;
		_dtb arguments = { 0 };
		arguments.process_id = process_id;
		arguments.operation = (bool*)&ret;
		DeviceIoControl(driver_handle, dtbfix_code, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);

		return ret;
	}
	inline INT32 find_process(LPCTSTR process_name) {
		PROCESSENTRY32 pt;
		HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		pt.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hsnap, &pt)) {
			do {
				if (!lstrcmpi(pt.szExeFile, process_name)) {
					CloseHandle(hsnap);
					process_id = pt.th32ProcessID;
					return pt.th32ProcessID;
				} 
			} while (Process32Next(hsnap, &pt));
		}
		CloseHandle(hsnap);

		return { NULL };
	}

	template <typename T>
	inline T read(uint64_t address, std::string stringRead) {
		T buffer{ };
		mem::read_physical((PVOID)address, &buffer, sizeof(T));
		return buffer;
	}
	inline bool IsValidPointer(uint64_t Pointer) {
		if (Pointer <= 0x400000 || Pointer == 0xCCCCCCCCCCCCCCCC || reinterpret_cast<void*>(Pointer) == nullptr || Pointer >
			0x7FFFFFFFFFFFFFFF) {
			return false;
		}
		return true;
	}
	inline void read(uint64_t address, PVOID buffer, DWORD size) {
		read_physical((PVOID)address, buffer, size);
	}

	inline void Read(uint64_t address, PVOID buffer, DWORD size) {
		read_physical((PVOID)address, buffer, size);
	}

	template <typename T>
	inline T Read(uint64_t address, const char* whatAreYouReading) {
		T buffer{ };
		mem::read_physical((PVOID)address, &buffer, sizeof(T));
		return buffer;
	}

	template <typename T>
	inline T write(uint64_t address, T buffer) {
		mem::write_physical((PVOID)address, &buffer, sizeof(T));
		return buffer;
	}
	inline bool is_valid(const uint64_t adress)
	{
		if (adress <= 0x400000 || adress == 0xCCCCCCCCCCCCCCCC || reinterpret_cast<void*>(adress) == nullptr || adress >
			0x7FFFFFFFFFFFFFFF) {
			return false;
		}
		return true;
	}
	inline std::string readString(uint64_t address, DWORD size, const char* whatAreYouReading) {
		char* buffer = new char[size];
		read(address, buffer, size);
		auto str = std::string(buffer);
		delete[] buffer;
		return str;
	}
	inline std::string ReadString(uint64_t address, DWORD size, const char* whatAreYouReading) {
		char buffer[128] = { 0 };
		read(address, &buffer, 128);
		auto str = std::string(buffer);
		return str;
	}
}

