#pragma once
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <random>
#include "nt.hpp"
#include "driver_data.hpp"

#define Log(content) std::wcout << content

class loader
{
private:
	std::wstring driver_name;
	std::wstring driver_path;
public:
	loader()
	{
		driver_name = generate_random_wstring(12);
		std::wstring temp = GetFullTempPath();
		if (temp.empty())
		{
			driver_path = L"";
		}
		driver_path = temp + L"\\" + driver_name;
	}

	std::wstring generate_random_wstring(size_t length, const std::wstring& charset = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") {
		std::random_device rd; // 用于种子
		std::mt19937 generator(rd()); // 生成器
		std::uniform_int_distribution<> dist(0, charset.size() - 1); // 均匀分布

		std::wstring random_wstring;
		for (size_t i = 0; i < length; ++i) {
			random_wstring += charset[dist(generator)];
		}

		return random_wstring;
	}

	std::wstring GetFullTempPath() {
		wchar_t temp_directory[MAX_PATH + 1] = { 0 };
		const uint32_t get_temp_path_ret = GetTempPathW(sizeof(temp_directory) / 2, temp_directory);
		if (!get_temp_path_ret || get_temp_path_ret > MAX_PATH + 1) {
			Log(L"[-] Failed to get temp path" << std::endl);
			return L"";
		}
		if (temp_directory[wcslen(temp_directory) - 1] == L'\\')
			temp_directory[wcslen(temp_directory) - 1] = 0x0;

		return std::wstring(temp_directory);
	}

	bool CreateFileFromMemory(const char* address, size_t size) {
		std::ofstream file_ofstream(driver_path.c_str(), std::ios_base::out | std::ios_base::binary);

		if (!file_ofstream.write(address, size)) {
			file_ofstream.close();
			return false;
		}

		file_ofstream.close();
		return true;
	}

	bool RegisterAndStart() {
		const static DWORD ServiceTypeKernel = 1;
		const std::wstring servicesPath = L"SYSTEM\\CurrentControlSet\\Services\\" + driver_name;
		const std::wstring nPath = L"\\??\\" + driver_path;

		HKEY dservice;
		LSTATUS status = RegCreateKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &dservice); //Returns Ok if already exists
		if (status != ERROR_SUCCESS) {
			Log("[-] Can't create service key" << std::endl);
			return false;
		}

		status = RegSetKeyValueW(dservice, NULL, L"ImagePath", REG_EXPAND_SZ, nPath.c_str(), (DWORD)(nPath.size() * sizeof(wchar_t)));
		if (status != ERROR_SUCCESS) {
			RegCloseKey(dservice);
			Log("[-] Can't create 'ImagePath' registry value" << std::endl);
			return false;
		}

		status = RegSetKeyValueW(dservice, NULL, L"Type", REG_DWORD, &ServiceTypeKernel, sizeof(DWORD));
		if (status != ERROR_SUCCESS) {
			RegCloseKey(dservice);
			Log("[-] Can't create 'Type' registry value" << std::endl);
			return false;
		}

		RegCloseKey(dservice);

		HMODULE ntdll = GetModuleHandleA("ntdll.dll");
		if (ntdll == NULL) {
			return false;
		}

		auto RtlAdjustPrivilege = (nt::RtlAdjustPrivilege)GetProcAddress(ntdll, "RtlAdjustPrivilege");
		auto NtLoadDriver = (nt::NtLoadDriver)GetProcAddress(ntdll, "NtLoadDriver");

		ULONG SE_LOAD_DRIVER_PRIVILEGE = 10UL;
		BOOLEAN SeLoadDriverWasEnabled;
		NTSTATUS Status = RtlAdjustPrivilege(SE_LOAD_DRIVER_PRIVILEGE, TRUE, FALSE, &SeLoadDriverWasEnabled);
		if (!NT_SUCCESS(Status)) {
			Log("Fatal error: failed to acquire SE_LOAD_DRIVER_PRIVILEGE. Make sure you are running as administrator." << std::endl);
			return false;
		}

		std::wstring wdriver_reg_path = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + driver_name;
		UNICODE_STRING serviceStr;
		RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

		Status = NtLoadDriver(&serviceStr);
		Log("[+] NtLoadDriver Status 0x" << std::hex << Status << std::endl);

		//Never should occur since kdmapper checks for "IsRunning" driver before
		if (Status == 0xC000010E) {// STATUS_IMAGE_ALREADY_LOADED
			return true;
		}
		return NT_SUCCESS(Status);
	}

	bool Load() {
		const HANDLE file_handle = CreateFileW(L"\\\\.\\BLoader", FILE_ANY_ACCESS, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file_handle != nullptr && file_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file_handle);
			return true;
		}

		Log(L"[<] Loading vulnerable driver, Name: " << driver_name << std::endl);

		if (driver_path.empty()) {
			Log(L"[-] Can't find TEMP folder" << std::endl);
			return false;
		}

		_wremove(driver_path.c_str());

		if (!CreateFileFromMemory(reinterpret_cast<const char*>(driver_data), sizeof(driver_data))) {
			Log(L"[-] Failed to create vulnerable driver file" << std::endl);
			return false;
		}

		if (!RegisterAndStart()) {
			Log(L"[-] Failed to register and start service for the vulnerable driver" << std::endl);
			_wremove(driver_path.c_str());
			return false;
		}

		return true;
	}

	bool StopAndRemove() {
		HMODULE ntdll = GetModuleHandleA("ntdll.dll");
		if (ntdll == NULL)
			return false;

		std::wstring wdriver_reg_path = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + driver_name;
		UNICODE_STRING serviceStr;
		RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

		HKEY driver_service;
		std::wstring servicesPath = L"SYSTEM\\CurrentControlSet\\Services\\" + driver_name;
		LSTATUS status = RegOpenKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &driver_service);
		if (status != ERROR_SUCCESS) {
			if (status == ERROR_FILE_NOT_FOUND) {
				return true;
			}
			return false;
		}
		RegCloseKey(driver_service);

		auto NtUnloadDriver = (nt::NtUnloadDriver)GetProcAddress(ntdll, "NtUnloadDriver");
		NTSTATUS st = NtUnloadDriver(&serviceStr);
		Log("[+] NtUnloadDriver Status 0x" << std::hex << st << std::endl);
		if (st != 0x0) {
			Log("[-] Driver Unload Failed!!" << std::endl);
			status = RegDeleteTreeW(HKEY_LOCAL_MACHINE, servicesPath.c_str());
			return false; //lets consider unload fail as error because can cause problems with anti cheats later
		}


		status = RegDeleteTreeW(HKEY_LOCAL_MACHINE, servicesPath.c_str());
		if (status != ERROR_SUCCESS) {
			return false;
		}
		return true;
	}

	bool Unload() {
		if (!StopAndRemove())
			return false;

		//Destroy disk information before unlink from disk to prevent any recover of the file
		std::ofstream file_ofstream(driver_path.c_str(), std::ios_base::out | std::ios_base::binary);
		int newFileLen = sizeof(driver_data) + (((long long)rand() * (long long)rand()) % 2000000 + 1000);
		BYTE* randomData = new BYTE[newFileLen];
		for (size_t i = 0; i < newFileLen; i++) {
			randomData[i] = (BYTE)(rand() % 255);
		}
		if (!file_ofstream.write((char*)randomData, newFileLen)) {
			Log(L"[!] Error dumping shit inside the disk" << std::endl);
		}
		else {
			Log(L"[+] Vul driver data destroyed before unlink" << std::endl);
		}
		file_ofstream.close();
		delete[] randomData;

		//unlink the file
		if (_wremove(driver_path.c_str()) != 0)
			return false;

		return true;
	}
};