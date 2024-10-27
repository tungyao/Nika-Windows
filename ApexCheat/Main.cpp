#define IMGUI_DEFINE_MATH_OPERATORS
//#define _KERNEL_MODE
#include "includes.hpp"
#pragma comment(lib, "Winmm.lib")
#pragma warning(disable : 4005, 4244)
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/byte_array.h"
#include "Inject.h"
#include <thread>
#include "vals.hpp"
#include "Overlay/Overlay.hpp"
using namespace std;

// fonts
ImFont* mainfont;
ImFont* smallfont;
ImFont* iconfont;
ImFont* logofont;

enum tabs
{
	aimbot,
	glow,
	esp,
	inject,
	other,
	key
};

tabs tabmenu = aimbot;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool particles = true;
DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs;
MyDisplay* myDisplay = new MyDisplay();
Overlay overlayWindow = Overlay();

Level* theMap = new Level();
LocalPlayer* localPlayer = new LocalPlayer();
std::vector<Player*>* humanPlayers = new std::vector<Player*>;
std::vector<Player*>* dummyPlayers = new std::vector<Player*>;
std::vector<Player*>* players = new std::vector<Player*>;
std::vector<Player*>* playersCache = new std::vector<Player*>;

Camera* gameCamera = new Camera();
ImDrawList* canvas;
// create features
AimBot* aimBot = new AimBot(myDisplay, localPlayer, players, gameCamera);
Sense* sense = new Sense(localPlayer, players, gameCamera, aimBot);
int spec;
int readError = 1000;

int counter = 1;
bool leftLock = 1;
bool rightLock = AIMBOT_ACTIVATED_BY_ADS;
bool autoFire = FEATURE_TRIGGERBOT_ON;
int boneId = 2;
int processingTime;
std::vector<int> processingTimes;
double averageProcessingTime;
int frameCountPrev;
std::vector<int> frameCountDiffs;
std::vector<int> frameCountTimes;
double averageFps;
int cache = 0;
int totalSpectators = 0;
std::vector<std::string> spectators;
void renderMenu();

void renderUI() {
	auto io = ImGui::GetIO();
	ImGui::SetNextWindowSize(io.DisplaySize);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("##Overlay", nullptr,
		ImGuiSliderFlags_AlwaysClamp |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoInputs);
	canvas = ImGui::GetWindowDrawList();
	if (readError > 0) {
		sense->renderStatus(leftLock, rightLock, autoFire, boneId, 0.0f, 0.0f, 0);
	}
	else {
		sense->renderStatus(leftLock, rightLock, autoFire, boneId, averageProcessingTime, averageFps, cache);
		sense->renderESP(canvas);
		if (FEATURE_MAP_RADAR_ON) sense->renderRadar(canvas);
		if (FEATURE_SPECTATORS_ON) sense->renderSpectators(totalSpectators, spectators);
	}
	ImGui::End();
	if (keymap::SHOW_MENU)
	{
		renderMenu();
	}
}

void renderMenu() {
	ImGui::Begin("Menu");
	ImGui::SetNextWindowSize(ImVec2(300, 800));
	ImGui::Checkbox("Aimbot on", &FEATURE_AIMBOT_ON);
	ImGui::Checkbox("Triggerbot on", &FEATURE_TRIGGERBOT_ON);
	ImGui::Checkbox("Spectators on", &FEATURE_SPECTATORS_ON);
	ImGui::Checkbox("Spectators show dead", &FEATURE_SPECTATORS_SHOW_DEAD);
	ImGui::Checkbox("Super glide on", &FEATURE_SUPER_GLIDE_ON);
	ImGui::Checkbox("Map radar on", &FEATURE_MAP_RADAR_ON);
	ImGui::Checkbox("Show fov", &SENSE_SHOW_FOV);
	ImGui::Checkbox("Show target", &SENSE_SHOW_TARGET);

	ImGui::SliderInt("Triggerbot hipfire range", &TRIGGERBOT_HIPFIRE_RANGE, 10, 200);
	ImGui::SliderInt("Esp range", &SENSE_MAX_RANGE, 100, 600);
	ImGui::Checkbox("Show player bar", &SENSE_SHOW_PLAYER_BARS);
	ImGui::Checkbox("Show player distance", &SENSE_SHOW_PLAYER_DISTANCES);
	ImGui::Checkbox("Show player name", &SENSE_SHOW_PLAYER_NAMES);
	ImGui::Checkbox("Show player level", &SENSE_SHOW_PLAYER_LEVELS);
	ImGui::Checkbox("Sense text bottom", &SENSE_TEXT_BOTTOM);
	ImGui::Checkbox("Sense show dead", &SENSE_SHOW_DEAD);


	ImGui::Checkbox("Aimbot activated by ads", &AIMBOT_ACTIVATED_BY_ADS);
	ImGui::Checkbox("Aimbot activated by key", &AIMBOT_ACTIVATED_BY_KEY);
	ImGui::Checkbox("Aimbot activated by mouse", &AIMBOT_ACTIVATED_BY_MOUSE);
	ImGui::Checkbox("Aimbot predict bulletdrop", &AIMBOT_PREDICT_BULLETDROP);
	ImGui::Checkbox("Aimbot predict movement", &AIMBOT_PREDICT_MOVEMENT);
	ImGui::Checkbox("Aimbot allow target switch", &AIMBOT_ALLOW_TARGET_SWITCH);
	ImGui::Checkbox("Aimbot friendly fire", &AIMBOT_FRIENDLY_FIRE);
	ImGui::SliderFloat("Aimbot speed", &AIMBOT_SPEED, 5, 300);
	ImGui::SliderFloat("Aimbot smooth", &AIMBOT_SMOOTH, 5, 300);
	ImGui::SliderFloat("Aimbot fov", &AIMBOT_FOV, 5, 100);
	ImGui::SliderFloat("Aimbot fov extra zoom", &AIMBOT_FOV_EXTRA_BY_ZOOM, 0.1, 2);
	ImGui::SliderInt("Aimbot max distance", &AIMBOT_MAX_DISTANCE, 10, 600);
	ImGui::SliderInt("Aimbot zoomed max move", &AIMBOT_ZOOMED_MAX_MOVE, 10, 200);
	ImGui::SliderInt("Aimbot max delta", &AIMBOT_MAX_DELTA, 0, 100);
	ImGui::SliderFloat("Aimbot fast area", &AIMBOT_FAST_AREA, 0.1, 5);
	ImGui::SliderFloat("Aimbot slow area", &AIMBOT_SLOW_AREA, 0.1, 5);
	ImGui::SliderFloat("Aimbot smooth extra distance", &AIMBOT_SMOOTH_EXTRA_BY_DISTANCE, 1000, 5000);


	ImGui::End();
}

#include <numeric>
#include "Inject.h"
#include "skCrypter.h"
using namespace skc;

DWORD WINAPI StartCheat(LPVOID lpParamter) {

	Inject();
	is_inject = mem::find_driver();
	while (is_inject == false)
	{
		printf("NOT OPEN DRIVER!\n");
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
#ifdef _DX12
	while (mem::find_process(TEXT("r5apex_dx12.exe")) == 0) {
#else
	while (mem::find_process(TEXT("r5apex.exe")) == 0) {
#endif

		printf("OPEN APEX LEGENDS!\n");

		std::this_thread::sleep_for(std::chrono::seconds(5));

	}

	while (!mem::CR3())
	{
		printf("NOT CR3\n");
		std::this_thread::sleep_for(std::chrono::seconds(5));

	}
	OFF_REGION = mem::find_image();
	std::cout << std::hex << "REGION: " << OFF_REGION << "\n";
	timeBeginPeriod(1);

	for (int i = 0; i < 70; i++) humanPlayers->push_back(new Player(i));
	for (int i = 0; i < 15000; i++) dummyPlayers->push_back(new Player(i));

	auto overlayName = OBF("chrome");
	if (SENSE_VERBOSE == 2) {
		if (!overlayWindow.InitializeOverlay(overlayName)) {
			overlayWindow.DestroyOverlay();
			return false;
		}

		overlayWindow.SetStyle();
		overlayWindow.CaptureInput(false);
		int screenWidth;
		int screenHeight;
		overlayWindow.GetScreenResolution(screenWidth, screenHeight);
		gameCamera->initialize(screenWidth, screenHeight);
		std::cout << "Overlay Initialized!" << std::endl;
	}
	else {
		gameCamera->initialize(2560, 1440);
	}

	int counter = 0;

	while (true) {
		try {
			uint64_t startTime = util::currentEpochMillis();
			if (readError > 0) {
				myDisplay->kbRelease(AIMBOT_FIRING_KEY);
				keymap::AIMBOT_FIRING_KEY = false;
				playersCache->clear();

			}

			while (true) {

				if (readError > 0) {
					if (SENSE_VERBOSE == 2) overlayWindow.Render(&renderUI);
					util::sleep(50);
					readError -= 50;
				}
				else { break; }
			}

			// Read map and make sure it is playable
			theMap->readFromMemory();
			if (!theMap->isPlayable) {
				std::cout << "Player in Lobby - Sleep 3 sec\n";
				readError = 3000;
				continue;
			}

			// Read localPlayer and make sure it is valid
			localPlayer->readFromMemory();
			if (!localPlayer->isValid()) {
				std::cout << "Select Legend - Sleep 3 sec\n";
				readError = 3000;
				continue;
			}

			if (AIMBOT_ACTIVATED_BY_KEY && (AIMBOT_ACTIVATION_KEY != 0 || "NONE") && myDisplay->isKeyDown(AIMBOT_ACTIVATION_KEY) ||
				AIMBOT_ACTIVATED_BY_MOUSE && myDisplay->isLeftMouseButtonDown())
				keymap::AIMBOT_ACTIVATION_KEY = true;
			else
				keymap::AIMBOT_ACTIVATION_KEY = false;
			if (myDisplay->isKeyDown(SHOW_MENU_KEY)) {
				keymap::SHOW_MENU = !keymap::SHOW_MENU;
				if (SENSE_VERBOSE == 2) overlayWindow.CaptureInput(keymap::SHOW_MENU);
				util::sleep(100);
			}
			
			int weapon = localPlayer->weaponId;
			if (AIMBOT_ACTIVATED_BY_MOUSE && myDisplay->isLeftMouseButtonDown() && (
				weapon == WEAPON_SENTINEL ||
				weapon == WEAPON_LONGBOW ||
				weapon == WEAPON_KRABER ||
				weapon == WEAPON_TRIPLE_TAKE)) {
				std::chrono::milliseconds timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
				if (timeNow > keymap::timeLastShot + std::chrono::milliseconds(125)) {
					myDisplay->kbPress(AIMBOT_FIRING_KEY);
					myDisplay->kbRelease(AIMBOT_FIRING_KEY);
					keymap::AIMBOT_FIRING_KEY = false;
					keymap::timeLastShot = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
				}
			}

			// Read players
			players->clear();
			if (counter % 99 == 0) {
				playersCache->clear();

				for (int i = 0; i < dummyPlayers->size(); i++) {
					Player* p = dummyPlayers->at(i);
					p->readFromMemory(theMap, localPlayer, counter);
					if (p->isValid()) { playersCache->push_back(p); players->push_back(p); }
				}


				cache = playersCache->size();
				printf("Entities: %d\n", cache);

				if (FEATURE_SPECTATORS_ON && SENSE_VERBOSE == 2) {
					int tempTotalSpectators = 0;
					std::vector<std::string> tempSpectators;
					for (int i = 0; i < players->size(); i++) {
						Player* p = players->at(i);
						if (p->base == localPlayer->base)
							continue;
						if (p->spctrBase == localPlayer->base) {
							tempTotalSpectators++;
							tempSpectators.push_back(p->getPlayerName());
						}
						else if (FEATURE_SPECTATORS_SHOW_DEAD && p->isDead) {
							tempTotalSpectators++;
							tempSpectators.push_back("DEAD: " + p->getPlayerName());
						}
					}
					totalSpectators = tempTotalSpectators;
					spectators = tempSpectators;
					printf("Spectators: %d\n", static_cast<int>(spectators.size()));
					if (static_cast<int>(spectators.size()) > 0)
						for (int i = 0; i < static_cast<int>(spectators.size()); i++)
							printf("> %s\n", spectators.at(i).c_str());
				}

			}
			else {
				for (int i = 0; i < playersCache->size(); i++) {
					Player* p = playersCache->at(i);
					p->readFromMemory(theMap, localPlayer, counter);
					if (p->isValid()) players->push_back(p);
				}
			}

			// Run features
			gameCamera->update();
			aimBot->update(leftLock, rightLock, autoFire, boneId, totalSpectators);


			if (SENSE_VERBOSE == 2) overlayWindow.Render(&renderUI);
			if (SENSE_VERBOSE == 3) sense->update(counter);
			processingTime = static_cast<int>(util::currentEpochMillis() - startTime);
			int goalSleepTime = 6.04; // 16.67ms=60Hz | 6.94ms=144Hz
			int timeLeftToSleep = (std::max)(0, goalSleepTime - processingTime);
			util::sleep(timeLeftToSleep);
			if (counter % 100 == 0) {
				if (SENSE_VERBOSE > 0) {
					printf("%d %d %d ", leftLock, autoFire, rightLock);
					if (boneId == 0) printf("HEAD\n");
					else if (boneId == 1) printf("NECK\n");
					else printf("BODY\n");
				}
				processingTimes.push_back(processingTime);
				if (processingTimes.size() > 10) processingTimes.erase(processingTimes.begin());
				averageProcessingTime = std::accumulate(processingTimes.begin(), processingTimes.end(), 0.0f) / processingTimes.size();
				if (FEATURE_SUPER_GLIDE_ON) {
					if (frameCountPrev != 0) {
						frameCountDiffs.push_back(localPlayer->frameCount - frameCountPrev);
						frameCountTimes.push_back(static_cast<int>(util::currentEpochMillis() - startTime));
					}
					if (frameCountDiffs.size() > 10)
						frameCountDiffs.erase(frameCountDiffs.begin());
					if (frameCountTimes.size() > 10)
						frameCountTimes.erase(frameCountTimes.begin());
					averageFps = std::accumulate(frameCountDiffs.begin(), frameCountDiffs.end(), 0.0f) / std::accumulate(frameCountTimes.begin(), frameCountTimes.end(), 0.0f) * 10;
					frameCountPrev = localPlayer->frameCount;
				}
			}

			// Print loop info every now and then
			if (SENSE_VERBOSE > 0 && counter % 500 == 0)
				printf("| [%04d] - Time: %02dms |\n", counter, processingTime);

			// Update counter
			counter = (counter >= 9999) ? counter = 0 : ++counter;
		}
		catch (std::invalid_argument& e) {
			printf("[-] %s - SLEEP 5 SEC [-]\n", e.what());
			readError = 5000;
		}
		catch (...) {
			printf("[-] UNKNOWN ERROR - SLEEP 3 SEC [-]\n");
			readError = 3000;
		}
	}
	return 0L;
}

int wmain()
{

	StartCheat(0);
}
