#pragma once
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <mutex>
#include <Windows.h> // Windows API 头文件
#include <WinUser.h>

// 鼠标控制器
class MouseController {
private:
    double deltaX, deltaY; // 当前增量
    bool hasDelta;         // 用于判断是否有增量
    std::mutex mtx;

public:
    MouseController() : deltaX(0), deltaY(0), hasDelta(false) {}

    // 更新增量
    void updateDelta(double x, double y) {
        std::lock_guard<std::mutex> lock(mtx);
        deltaX = x;
        deltaY = y;
        hasDelta = true; // 当有新的增量时，设置为 true
    }

    // 移动鼠标
    void moveMouse() {
        double interval = 0.01; // 采样间隔 (秒)
        double decayFactor = 0.9; // 衰减因子，用于控制增量逐步减小

        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx);

                // 如果没有增量，跳过此次循环
                if (!hasDelta) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                // 使用 SendInput 发送鼠标移动指令
                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dx = static_cast<int>(deltaX); // 当前水平移动
                input.mi.dy = static_cast<int>(deltaY); // 当前垂直移动
                input.mi.dwFlags = MOUSEEVENTF_MOVE;

                SendInput(1, &input, sizeof(INPUT));

                // 输出当前移动增量
                std::cout << "Moving mouse by delta: (" << deltaX << ", " << deltaY << ")\n";

                // 衰减增量，逐渐减小到 0
                deltaX *= decayFactor;
                deltaY *= decayFactor;

                // 如果增量非常小，停止移动
                if (std::abs(deltaX) < 0.5 && std::abs(deltaY) < 0.5) {
                    hasDelta = false; // 停止移动
                    deltaX = 0;
                    deltaY = 0;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};

inline MouseController mouseController; // PID 参数和持续时间 (1秒)

DWORD WINAPI StartPID(LPVOID lpParamter);