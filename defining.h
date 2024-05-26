#pragma once
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH			640
#define SCREEN_HEIGHT			480
#define AREA_WIDTH				1920
#define AREA_HEIGHT				1080
#define PLAYER_SPEED_UNIT		1
#define PLAYER_SPEED_MULTIPLIER	4
#define CAMERA_SPEED			4
#define PLAYER_BULLET_SPEED		16
#define DAMAGE_DEALT			1
#define DAMAGE_MULTIPLIER		5
#define ANIMATION_FRAMES_TOTAL	18
#define PILLS_AMOUNT			5
#define HUDX					460
