#pragma once
#include "Offsets.hpp"
#include "obfusheader.hpp"


struct Camera {
	Vector2D screenSize;
	ViewMatrix gameViewMatrix;

	void initialize(int width, int height) {
		screenSize = Vector2D(width, height);
	}

	const Vector2D& getResolution() {
		return screenSize;
	}

	void update() {
		uint64_t renderPtr = mem::Read<uint64_t>(OFF_REGION + OFF_VIEW_RENDER, OBF("Camera renderPtr"));
		uint64_t matrixPtr = mem::Read<uint64_t>(renderPtr + OFF_VIEW_MATRIX, OBF("Camera matrixPtr"));
		gameViewMatrix = mem::Read<ViewMatrix>(matrixPtr, OBF("Camera gameViewMatrix"));
	}

	bool worldToScreen(Vector3D worldPosition, Vector2D& screenPosition) const {
		Vector3D transformed = gameViewMatrix.Transform(worldPosition);

		if (transformed.z < 0.001f) {
			return false;
		}

		transformed.x *= 1.0f / transformed.z;
		transformed.y *= 1.0f / transformed.z;

		screenPosition = Vector2D(screenSize.x / 2.0f + transformed.x * (screenSize.x / 2.0f), screenSize.y / 2.0f - transformed.y * (screenSize.y / 2.0f));

		return true;
	}
};
