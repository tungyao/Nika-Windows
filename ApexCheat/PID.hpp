#pragma once
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <mutex>
#include <Windows.h> // Windows API ͷ�ļ�
#include <WinUser.h>

// ��������
class MouseController {
private:
    double deltaX, deltaY; // ��ǰ����
    bool hasDelta;         // �����ж��Ƿ�������
    std::mutex mtx;

public:
    MouseController() : deltaX(0), deltaY(0), hasDelta(false) {}

    // ��������
    void updateDelta(double x, double y) {
        std::lock_guard<std::mutex> lock(mtx);
        deltaX = x;
        deltaY = y;
        hasDelta = true; // �����µ�����ʱ������Ϊ true
    }

    // �ƶ����
    void moveMouse() {
        double interval = 0.01; // ������� (��)
        double decayFactor = 0.9; // ˥�����ӣ����ڿ��������𲽼�С

        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx);

                // ���û�������������˴�ѭ��
                if (!hasDelta) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                // ʹ�� SendInput ��������ƶ�ָ��
                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dx = static_cast<int>(deltaX); // ��ǰˮƽ�ƶ�
                input.mi.dy = static_cast<int>(deltaY); // ��ǰ��ֱ�ƶ�
                input.mi.dwFlags = MOUSEEVENTF_MOVE;

                SendInput(1, &input, sizeof(INPUT));

                // �����ǰ�ƶ�����
                std::cout << "Moving mouse by delta: (" << deltaX << ", " << deltaY << ")\n";

                // ˥���������𽥼�С�� 0
                deltaX *= decayFactor;
                deltaY *= decayFactor;

                // ��������ǳ�С��ֹͣ�ƶ�
                if (std::abs(deltaX) < 0.5 && std::abs(deltaY) < 0.5) {
                    hasDelta = false; // ֹͣ�ƶ�
                    deltaX = 0;
                    deltaY = 0;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};

inline MouseController mouseController; // PID �����ͳ���ʱ�� (1��)

DWORD WINAPI StartPID(LPVOID lpParamter);