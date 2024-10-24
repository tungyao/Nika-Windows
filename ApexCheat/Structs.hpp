#pragma once
#include "obfusheader.hpp"


struct Level {
	std::string name;
	bool isLobby;
	bool isPlayable;
	bool isTrainingArea;
	char gameMode[64] = { 0 };
	std::unordered_map<std::string, bool> gameModes = { {"control", true}, {"freedm", true} };
	bool isMixtape;

	void readFromMemory() {
		name = mem::ReadString(OFF_REGION + OFF_LEVEL_NAME, 1024, OBF("Level name"));
		isLobby = name == "mp_lobby";
		isPlayable = !name.empty() && name != "mp_lobby";
		isTrainingArea = name == "mp_rr_canyonlands_staging_mu1";
		uint64_t gameModePtr = mem::Read<uint64_t>(OFF_REGION + OFF_GAME_MODE + 0x50, OBF("Level gameModePtr"));
		if (gameModePtr > 0) {
			mem::Read(gameModePtr, &gameMode, sizeof(gameMode));
			isMixtape = gameModes[gameMode];
		}
	}
};

namespace keymap {
	bool AIMBOT_ACTIVATION_KEY;
	bool AIMBOT_FIRING_KEY;
	bool SHOW_MENU;
	static std::chrono::milliseconds timeLastShot;
};
namespace math {
	double distanceToMeters(float distance)
	{
		const float INCHES_TO_METER_RATE = 39.3701;
		return distance / INCHES_TO_METER_RATE;
	}
	double calculateDistance(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		float dx = (x1 - x2);
		float dy = (y1 - y2);
		float dz = (z1 - z2);
		float distance = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
		return distance;
	}

	double calculateDistanceInMeters(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		return distanceToMeters(calculateDistance(x1, y1, z1, x2, y2, z2));
	}

	double calculateDistance2D(float x1, float y1, float x2, float y2)
	{
		float dx = (x1 - x2);
		float dy = (y1 - y2);
		float distance = sqrt(pow(dx, 2) + pow(dy, 2));
		return distance;
	}

};

namespace util {


	void sleep(int ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	uint64_t currentEpochMillis() {
		auto currentTime = std::chrono::system_clock::now();
		auto duration = currentTime.time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	}

	float inchesToMeters(float inches) {
		return inches / 39.37007874;
	}

	float metersToInches(float meters) {
		return 39.37007874 * meters;
	}

	float randomFloat(float min, float max) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}
};

enum class HitboxType {
	Head = 0,
	Neck = 1,
	UpperChest = 2,
	LowerChest = 3,
	Stomach = 4,
	Hip = 5,
	LeftShoulder = 6,
	LeftElbow = 7,
	LeftHand = 8,
	RightShoulder = 9,
	RightElbow = 10,
	RightHand = 11,
	LeftThighs = 12,
	LeftKnees = 13,
	LeftLeg = 18,
	RightThighs = 16,
	RightKnees = 17,
	RightLeg = 14
};
