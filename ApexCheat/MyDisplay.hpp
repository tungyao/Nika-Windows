#pragma once

#include <WinUser.h>
#include "PID.hpp"

struct MyDisplay {
public:
	MyDisplay() {
		// No need to initialize anything specific for Windows in this case.
	}

	void mouseClickLeft(int delay_ms) {
		INPUT input[2] = {};
		input[0].type = INPUT_MOUSE;
		input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput(1, &input[0], sizeof(INPUT));
		Sleep(delay_ms);
		input[1].type = INPUT_MOUSE;
		input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(1, &input[1], sizeof(INPUT));
	}

	void mouseClick(int button) {
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP; // For left button click
		if (button == VK_RBUTTON) {
			input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
		}
		else if (button == VK_MBUTTON) {
			input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP;
		}
		SendInput(1, &input, sizeof(INPUT));
	}

	void moveMouseRelative(int pitchMovement, int yawMovement) {
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_MOVE;
		input.mi.dx = yawMovement;
		input.mi.dy = pitchMovement;
		printf("x %d y %d\n", pitchMovement, pitchMovement);
		SendInput(1, &input, sizeof(INPUT));
		//mouseController.updateDelta(pitchMovement, yawMovement);
	}

	bool isLeftMouseButtonDown() {
		return (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
	}

	bool keyDown(int keyCode) {
		return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
	}

	bool keyDown(std::string keyName) {
		int vkCode = trimXKPrefix(keyName)[0];
		if (vkCode != 0) {
			return keyDown(vkCode);
		}
		return false;
	}
	bool isKeyDown(int vkCode) {

		if (vkCode != 0) {
			return keyDown(vkCode);
		}
		return false;
	}
	void mouse_left_down() {
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput(1, &input, sizeof(INPUT));
	}
	void mouse_left_up() {
		INPUT input = {};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(1, &input, sizeof(INPUT));
	}


	bool  kbPress(int keyCode) {
		INPUT input = {};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = keyCode;
		input.ki.dwFlags = 0; // KEYEVENTF_KEYDOWN
		SendInput(1, &input, sizeof(INPUT));
		return true;
	}
	bool kbRelease(int keyCode) {
		INPUT input = {};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = keyCode;
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &input, sizeof(INPUT));
		return true;
	}
	std::string trimXKPrefix(const std::string& keyName) {
		if (keyName.compare(0, 3, "XK_") == 0)
			return keyName.substr(3);
		return keyName;
	}

private:
	int keyNameToVKCode(const std::string& keyName) {
		if (keyName.length() != 1) {
			return -1; // Invalid key name for VkKeyScan.
		}
		return VkKeyScan(keyName[0]) & 0xFF;
	}
};
