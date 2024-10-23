#pragma once
// Features
inline bool FEATURE_AIMBOT_ON = true;
inline bool FEATURE_TRIGGERBOT_ON = true;
inline bool FEATURE_SPECTATORS_ON = true;
inline bool FEATURE_SPECTATORS_SHOW_DEAD = false;
inline bool FEATURE_SUPER_GLIDE_ON = false;
inline bool FEATURE_MAP_RADAR_ON = true;
inline int FEATURE_MAP_RADAR_X = 335;
inline int FEATURE_MAP_RADAR_Y = 335;
// TriggerBot
//  int TRIGGERBOT_ZOOMED_RANGE = 180;
inline int TRIGGERBOT_HIPFIRE_RANGE = 60;
// Sense
inline int SENSE_VERBOSE = 2;
inline int SENSE_MAX_RANGE = 300;
inline bool SENSE_SHOW_PLAYER_BARS = true;
inline bool SENSE_SHOW_PLAYER_DISTANCES = true;
inline bool SENSE_SHOW_PLAYER_NAMES = false;
inline bool SENSE_SHOW_PLAYER_LEVELS = false;
inline bool SENSE_TEXT_BOTTOM = true;
inline bool SENSE_SHOW_DEAD = false;
inline bool SENSE_SHOW_FOV = true;
inline bool SENSE_SHOW_TARGET = true;
// AimBot
inline bool AIMBOT_ACTIVATED_BY_ADS = false;
inline bool AIMBOT_ACTIVATED_BY_KEY = true;
inline bool AIMBOT_ACTIVATED_BY_MOUSE = true;
inline float AIMBOT_SPEED = 25;
inline float AIMBOT_SMOOTH = 40;
inline float AIMBOT_SMOOTH_EXTRA_BY_DISTANCE = 1500;
inline float AIMBOT_FOV = 7.5f;
inline float AIMBOT_FOV_EXTRA_BY_ZOOM = 0.2f;
inline float AIMBOT_FAST_AREA = 0.75f;
inline float AIMBOT_SLOW_AREA = 0.50f;
inline float AIMBOT_WEAKEN = 2;
inline bool AIMBOT_SPECTATORS_WEAKEN = true;
inline bool AIMBOT_PREDICT_BULLETDROP = true;
inline bool AIMBOT_PREDICT_MOVEMENT = true;
inline bool AIMBOT_ALLOW_TARGET_SWITCH = false;
inline bool AIMBOT_FRIENDLY_FIRE = false;
inline bool AIMBOT_LEGACY_MODE = false;
inline int AIMBOT_MAX_DISTANCE = 120;
inline int AIMBOT_MIN_DISTANCE = 0;
inline int AIMBOT_ZOOMED_MAX_MOVE = 30;
inline int AIMBOT_HIPFIRE_MAX_MOVE = 30;
inline int AIMBOT_MAX_DELTA = 10;

inline bool is_inject = false;

inline int AIMBOT_ACTIVATION_KEY = 88;
inline int AIMBOT_FIRING_KEY = 78;
inline int SHOW_MENU_KEY = 35;


inline float PERSON_GLOW_COLOR[3] = { 1.0f, 0.0f, 0.0f };

inline LPCWSTR DRIVERNAME = L"\\\\.\\Windowsloader";