#pragma once
#include "vals.hpp"
#include "obfusheader.hpp"


struct LocalPlayer {

	uint64_t base;
	int teamNumber;
	int squadNumber;
	Vector3D localOrigin;
	float worldTime;
	int currentHealth;
	bool isDead;
	bool isKnocked;
	int grenadeId;
	bool grippingGrenade;
	uint64_t highlightSettingsPtr;
	uint64_t weaponHandle;
	uint64_t weaponHandleMasked;
	uint64_t weaponEntity;
	int weaponId;
	float weaponProjectileSpeed;
	float weaponProjectileScale;
	float zoomFov;
	bool inZoom;
	Vector2D viewAngles;
	Vector3D cameraPosition;
	int frameCount;
	float traversalStartTime;
	float traversalProgress;

	LocalPlayer() {

	}

	void reset() {
		base = 0;
	}

	void readFromMemory() {
		base = mem::Read<uint64_t>(OFF_REGION + OFF_LOCAL_PLAYER, OBF("LocalPlayer base"));
		if (base == 0) return;
		teamNumber = mem::Read<int>(base + OFF_TEAM_NUMBER, OBF("LocalPlayer teamNumber"));
		squadNumber = mem::Read<int>(base + OFF_SQUAD_ID, OBF("LocalPlayer squadNumber"));
		localOrigin = mem::Read<Vector3D>(base + OFF_LOCAL_ORIGIN, OBF("LocalPlayer localOrigin"));
		worldTime = mem::Read<float>(base + OFF_TIME_BASE, OBF("LocalPlayer worldTime"));
		currentHealth = mem::Read<int>(base + OFF_HEALTH, OBF("LocalPlayer currentHealth"));
		isDead = mem::Read<short>(base + OFF_LIFE_STATE, OBF("LocalPlayer isDead")) > 0;
		isKnocked = mem::Read<short>(base + OFF_BLEEDOUT_STATE, OBF("LocalPlayer isKnocked")) > 0;
		highlightSettingsPtr = mem::read<uint64_t>(OFF_REGION + OFF_GLOW_HIGHLIGHTS, "LocalPlayer HiglightsSettingPtr");
		if (!isDead && !isKnocked) {
			grenadeId = mem::Read<int>(base + OFF_GRENADE_HANDLE, OBF("LocalPlayer grenadeId"));
			grippingGrenade = grenadeId == -251 ? true : false;
			weaponHandle = mem::Read<uint64_t>(base + OFF_WEAPON_HANDLE, OBF("LocalPlayer weaponHandle"));
			weaponHandleMasked = weaponHandle & 0xffff;
			weaponEntity = mem::Read<uint64_t>(OFF_REGION + OFF_ENTITY_LIST + (weaponHandleMasked << 5), OBF("LocalPlayer weaponEntity"));
			weaponId = mem::Read<int>(weaponEntity + OFF_WEAPON_INDEX, OBF("LocalPlayer weaponId"));
			weaponProjectileSpeed = mem::Read<float>(weaponEntity + OFF_PROJECTILE_SPEED, OBF("LocalPlayer weaponProjectileSpeed"));
			weaponProjectileScale = mem::Read<float>(weaponEntity + OFF_PROJECTILE_SCALE, OBF("LocalPlayer weaponProjectileScale"));
			zoomFov = mem::Read<float>(weaponEntity + OFF_ZOOM_FOV, OBF("LocalPlayer zoomFov"));
		}
		inZoom = mem::Read<short>(base + OFF_ZOOMING, OBF("LocalPlayer inZoom")) > 0;
		viewAngles = mem::Read<Vector2D>(base + OFF_VIEW_ANGLES, OBF("LocalPlayer viewAngles"));
		cameraPosition = mem::Read<Vector3D>(base + OFF_CAMERA_ORIGIN, OBF("LocalPlayer cameraPosition"));
		if (FEATURE_SUPER_GLIDE_ON) {
			frameCount = mem::Read<int>(OFF_REGION + OFF_GLOBAL_VARS + sizeof(double), OBF("LocalPlayer frameCount"));
			traversalStartTime = mem::Read<float>(base + OFF_TRAVERSAL_START_TIME, OBF("LocalPlayer traversalStartTime"));
			traversalProgress = mem::Read<float>(base + OFF_TRAVERSAL_PROGRESS, OBF("LocalPlayer traversalProgress"));
		}
	}

	bool isValid() {
		return base != 0;
	}

	bool isCombatReady() {
		if (!isValid() || isDead || isKnocked) return false;
		return true;
	}
};
