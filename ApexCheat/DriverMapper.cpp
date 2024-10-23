#include <iostream>
#include "loader.hpp"
#include <vector>
#include "../DriverLoader/drvrecode_eac.hpp"

loader load;

bool ReadFileContent(const std::string& filePath, std::vector<char>& buffer) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer.resize(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to read file: " << filePath << std::endl;
        return false;
    }

    file.close();
    return true;
}

int Inject2()
{

	
	if (!load.Load())
	{
		std::cout << "Failed to load map driver\n";
		std::cin.get();
		return 0;
	}

    // 定义设备路径
    const char* devicePath = "\\\\.\\BLoader";


    // 使用CreateFile函数打开设备
    HANDLE hDevice = CreateFileA(
        devicePath,          // 设备路径
        GENERIC_READ | GENERIC_WRITE,  // 读/写访问权限
        0,                   // 不共享
        NULL,                // 默认安全属性
        OPEN_EXISTING,       // 打开已存在的设备
        0,                   // 属性和标志
        NULL                 // 模板文件句柄
    );

    // 检查设备是否成功打开
    if (hDevice == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open device. Error: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Device opened successfully." << std::endl;



    // 发送IOCTL请求
    DWORD bytesReturned;
    DWORD64 ret = 0;

    BOOL success = DeviceIoControl(
        hDevice,
        0x221998,
        &drvrecode_eac,
        sizeof(drvrecode_eac),
        &ret,
        sizeof(ret),
        &bytesReturned,
        NULL
    );

    if (success) {
        std::cout << "IOCTL request succeeded, " << bytesReturned << " bytes returned." << std::endl;
    }
    else {
        std::cerr << "IOCTL request failed. Error: " << GetLastError() << std::endl;
    }
    // 关闭设备句柄
    CloseHandle(hDevice);
    std::cout << "Device closed." << std::endl;
    load.Unload();
	return 0;
}