#pragma once
#include "vals.hpp"
#include <skCrypter.h>
using namespace skc;
#include "Memory.h"

struct Player {
	LocalPlayer* llp;
	int index;
	uintptr_t nameOffset;

	uint64_t base;
	std::string name;
	int teamNumber;
	bool isPlayer;
	bool isDrone;
	bool isDummie;
	Vector3D localOrigin;
	Vector3D localOriginDiff;
	Vector3D localOriginPrev;
	Vector3D localOriginPrev2;
	Vector3D localOriginPrev3;
	Vector3D localOriginPrev4;
	float timeLocalOrigin;
	float timeLocalOriginDiff;
	float timeLocalOriginPrev;
	float timeLocalOriginPrev2;
	float timeLocalOriginPrev3;
	float timeLocalOriginPrev4;
	Vector3D absoluteVelocity;
	Vector2D viewAngles;
	float viewYaw;
	int plyrDataTable;
	uint64_t spectators;
	int spctrIndex;
	uint64_t spctrBase;
	bool isDead;
	bool isKnocked;
	int currentHealth;
	int currentShield;
	//int lastTimeAimedAt;
	//bool isAimedAt;
	//int lastTimeAimedAtPrev;
	float lastTimeVisible;
	bool isVisible;
	bool isLocal;
	bool isFriendly;
	bool isEnemy;
	float distanceToLocalPlayer;
	float distance2DToLocalPlayer;

	Player(int in_index) {
		this->index = in_index;
	}

	void reset() {
		base = 0;
	}

	bool isSameTeam(Level* map, LocalPlayer* lp) {
		if (map->isMixtape && lp->squadNumber == -1)
			return (teamNumber & 1) == (lp->teamNumber & 1);
		else
			return teamNumber == lp->teamNumber;
	}

	void readFromMemory(Level* map, LocalPlayer* lp, int counter) {
		llp = lp;
		base = mem::Read<uint64_t>(OFF_REGION + OFF_ENTITY_LIST + ((index + 1) << 5), OBF("Player base"));
		if (base == 0) return;
		name = mem::ReadString(base + OFF_NAME, 1024, OBF("Player name"));
		isPlayer = name == "player";
		isDrone = name == "drone_no_minimap_object";
		if (map->isTrainingArea) {
			teamNumber = mem::Read<int>(base + OFF_TEAM_NUMBER, OBF("Player teamNumber"));
			isDummie = teamNumber == 97;
		}
		else { isDummie = false; }
		if (!isPlayer && !isDrone && !isDummie) { reset(); return; }
		if (!map->isTrainingArea)
			teamNumber = mem::Read<int>(base + OFF_TEAM_NUMBER, OBF("Player teamNumber"));
		localOrigin = mem::Read<Vector3D>(base + OFF_LOCAL_ORIGIN, OBF("Player localOrigin"));
		timeLocalOrigin = lp->worldTime;
		//if (!isPlayer) {
		localOriginDiff = localOrigin.Subtract(localOriginPrev).Add(localOriginPrev.Subtract(localOriginPrev2)).Add(localOriginPrev2.Subtract(localOriginPrev3)).Add(localOriginPrev3.Subtract(localOriginPrev4));
		timeLocalOriginDiff = (timeLocalOrigin - timeLocalOriginPrev) + (timeLocalOriginPrev - timeLocalOriginPrev2) + (timeLocalOriginPrev2 - timeLocalOriginPrev3) + (timeLocalOriginPrev3 - timeLocalOriginPrev4);
		absoluteVelocity = localOriginDiff.Divide(timeLocalOriginDiff); // v = d/t
		localOriginPrev4 = localOriginPrev3;
		localOriginPrev3 = localOriginPrev2;
		localOriginPrev2 = localOriginPrev;
		localOriginPrev = localOrigin;
		timeLocalOriginPrev4 = timeLocalOriginPrev3;
		timeLocalOriginPrev3 = timeLocalOriginPrev2;
		timeLocalOriginPrev2 = timeLocalOriginPrev;
		timeLocalOriginPrev = timeLocalOrigin;
		//} else {
		//    absoluteVelocity = mem::Read<Vector3D>(base + OFF_ABS_VELOCITY, "Player absoluteVelocity");
		if (isPlayer) {
			viewAngles = mem::Read<Vector2D>(base + OFF_VIEW_ANGLES, OBF("Player viewAngles"));
			viewYaw = mem::Read<float>(base + OFF_YAW, OBF("Player viewYaw"));
			plyrDataTable = mem::Read<int>(base + OFF_NAME_INDEX, OBF("Player plyrDataTable"));
			if (FEATURE_SPECTATORS_ON && counter % 99 == 0) {
				spectators = mem::Read<uint64_t>(OFF_REGION + OFF_OBSERVER_LIST, OBF("Player spectators"));
				spctrIndex = mem::Read<int>(spectators + plyrDataTable * 8 + OFF_OBSERVER_ARRAY, OBF("Player spctrIndex"));
				spctrBase = mem::Read<uint64_t>(OFF_REGION + OFF_ENTITY_LIST + ((spctrIndex & 0xFFFF) << 5), OBF("Player spctrBase"));
			}
		}
		isDead = (isDrone || isDummie) ? false : mem::Read<short>(base + OFF_LIFE_STATE, OBF("Player isDead")) > 0;
		isKnocked = (isDrone || isDummie) ? false : mem::Read<short>(base + OFF_BLEEDOUT_STATE, OBF("Player isKnocked")) > 0;
		currentHealth = mem::Read<int>(base + OFF_HEALTH, OBF("Player currentHealth"));
		currentShield = mem::Read<int>(base + OFF_SHIELD, OBF("Player currentShield"));

		//lastTimeAimedAt = mem::Read<int>(base + OFF_LAST_AIMEDAT_TIME, "Player lastTimeAimedAt");
		//isAimedAt = lastTimeAimedAtPrev < lastTimeAimedAt;
		//lastTimeAimedAtPrev = lastTimeAimedAt;

		lastTimeVisible = mem::Read<float>(base + OFF_LAST_VISIBLE_TIME, OBF("Player lastTimeVisible"));
		isVisible = (lastTimeVisible + 0.3f) > lp->worldTime || isDrone;

		if (lp->isValid()) {
			isLocal = base == lp->base;
			isFriendly = isSameTeam(map, lp);
			isEnemy = !isFriendly || AIMBOT_FRIENDLY_FIRE;// || map->isTrainingArea && isDrone;
			distanceToLocalPlayer = lp->localOrigin.Distance(localOrigin);
			distance2DToLocalPlayer = lp->localOrigin.Distance2D(localOrigin);
			//distance2DToLocalPlayer = lp->localOrigin.To2D().Distance(localOrigin.To2D());
		}
	}

	bool isValid() {
		return base != 0 && (isPlayer || isDrone || isDummie);
	}

	bool isCombatReady() {
		if (!isValid()) return false;
		if (isDrone || isDummie) return true;
		if (isDead || isKnocked) return false;
		return true;
	}

	std::string getPlayerName() {
		uintptr_t nameOffset = mem::Read<uintptr_t>(OFF_REGION + OFF_NAME_LIST + ((plyrDataTable - 1) * 24), OBF("Player nameOffset"));
		std::string playerName = mem::ReadString(nameOffset, 64, OBF("Player playerName"));
		return playerName;
	}

	int GetPlayerLevel() {
		int m_xp = mem::Read<int>(base + OFF_XP_LEVEL, OBF("Player m_xp"));
		if (m_xp < 0) return 0;
		if (m_xp < 100) return 1;

		int levels[] = { 2750, 6650, 11400, 17000, 23350, 30450, 38300, 46450, 55050,
		64100, 73600, 83550, 93950, 104800, 116100, 127850, 140050, 152400, 164900,
		177550, 190350, 203300, 216400, 229650, 243050, 256600, 270300, 284150, 298150,
		312300, 326600, 341050, 355650, 370400, 385300, 400350, 415550, 430900, 446400,
		462050, 477850, 493800, 509900, 526150, 542550, 559100, 575800, 592650, 609650,
		626800, 644100, 661550, 679150, 696900, 714800 };

		int level = 56;

		int arraySize = sizeof(levels) / sizeof(levels[0]);
		for (int i = 0; i < arraySize; i++)
			if (m_xp < levels[i])
				return i + 1;

		return level + ((m_xp - levels[arraySize - 1] + 1) / 18000);
	}

	int getBoneFromHitbox(HitboxType hitbox) const {
		uint64_t modelPointer = mem::Read<uint64_t>(base + OFF_STUDIO_HDR, OBF("Player modelPointer"));
		if (!mem::IsValidPointer(modelPointer))
			return -1;

		uint64_t studioHdr = mem::Read<uint64_t>(modelPointer + 0x8, OBF("Player studioHdr"));
		if (!mem::IsValidPointer(studioHdr + 0x34))
			return -1;

		auto hitboxCache = mem::Read<uint16_t>(studioHdr + 0x34, OBF("Player hitboxCache"));
		auto hitboxArray = studioHdr + ((uint16_t)(hitboxCache & 0xFFFE) << (4 * (hitboxCache & 1)));
		if (!mem::IsValidPointer(hitboxArray + 0x4))
			return -1;

		auto indexCache = mem::Read<uint16_t>(hitboxArray + 0x4, OBF("Player indexCache"));
		auto hitboxIndex = ((uint16_t)(indexCache & 0xFFFE) << (4 * (indexCache & 1)));
		auto bonePointer = hitboxIndex + hitboxArray + (static_cast<int>(hitbox) * 0x20);
		if (!mem::IsValidPointer(bonePointer))
			return -1;

		return mem::Read<uint16_t>(bonePointer, OBF("Player bonePointer"));
	}

	Vector3D getBonePosition(HitboxType hitbox) const {
		Vector3D offset = Vector3D(0.0f, 0.0f, 50.0f);

		int bone = getBoneFromHitbox(hitbox);
		if (bone < 0 || bone > 255)
			return localOrigin.Add(offset);

		uint64_t boneMatrixPtr = mem::Read<uint64_t>(base + OFF_BONE, OBF("Player boneMatrixPtr"));
		boneMatrixPtr += (bone * sizeof(Matrix3x4));
		if (!mem::IsValidPointer(boneMatrixPtr))
			return localOrigin.Add(offset);

		Matrix3x4 boneMatrix = mem::Read<Matrix3x4>(boneMatrixPtr, OBF("Player boneMatrix"));
		Vector3D bonePosition = boneMatrix.GetPosition();
		if (!bonePosition.IsValid())
			return localOrigin.Add(offset);

		return localOrigin.Add(bonePosition);
	}

	int getGlowThroughWall()
	{
		uint64_t ptrInt = mem::Read<uint64_t>(base + OFF_GLOW_THROUGH_WALL, OBF("Player GlowThroughWall"));
		if (!mem::is_valid(ptrInt))
			return -1;
		return ptrInt;
	}
	int getGlowEnable()
	{
		uint64_t ptrInt = mem::Read<uint64_t>(base + OFF_GLOW_HIGHLIGHT_ID, OBF("Player GlowEnable"));
		if (!mem::is_valid(ptrInt))
			return -1;
		return ptrInt;
	}

	void setGlowThroughWall(int glowThroughWall)
	{
		uint64_t ptrLong = base + OFF_GLOW_THROUGH_WALL;
		mem::write<int>(ptrLong, glowThroughWall);
	}
	void setGlowEnable(int glowEnable)
	{
		uint64_t ptrLong = base + OFF_GLOW_HIGHLIGHT_ID;
		mem::write<int>(ptrLong, glowEnable);
	}
	void setCustomGlow(int health, bool isVisible, bool isSameTeam)
	{
		static const int contextId = 0; // Same as glow enable
		uint64_t basePointer = base;
		int settingIndex = 50;
		std::array<unsigned char, 4> highlightFunctionBits = {
			2,   // InsideFunction
			125, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE
			64,  // OutlineRadius: size * 255 / 8
			64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7)
		};
		std::array<float, 3> glowColorRGB = { 0, 0, 0 };

		if (!isVisible) {

			settingIndex = 75;
			glowColorRGB = { 1.0f, 0.0f, 0.0f }; // red shield
		}
		else if (health >= 205) {
			settingIndex = 66;
			glowColorRGB = { 1, 0, 0 }; // red shield
		}
		else if (health >= 190) {
			settingIndex = 67;
			glowColorRGB = { 0.5, 0, 0.5 }; // purple shield
		}
		else if (health >= 170) {
			settingIndex = 68;
			glowColorRGB = { 0, 0.5, 1 }; // blue shield
		}
		else if (health >= 95) {
			settingIndex = 69;
			glowColorRGB = { 0, 1, 0.5 }; // gray shield // cyan color
		}
		else {
			settingIndex = 70;
			glowColorRGB = { 0, 1, 0 }; // low health enemies // green color
		}

		if (!isSameTeam) {
			mem::write<unsigned char>(basePointer + OFF_GLOW_HIGHLIGHT_ID + contextId, settingIndex);
			mem::write<decltype(highlightFunctionBits)>(
				llp->highlightSettingsPtr + HIGHLIGHT_TYPE_SIZE * settingIndex + 0, highlightFunctionBits);
			mem::write<decltype(glowColorRGB)>(
				llp->highlightSettingsPtr + HIGHLIGHT_TYPE_SIZE * settingIndex + 4, glowColorRGB);
			mem::write<int>(basePointer + OFF_GLOW_FIX, 2);
		}
	}
};
