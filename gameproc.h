#pragma once
#ifndef GAMEPROC_H
#define GAMEPROC_H
#include "duel.h"
#include <tchar.h>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <commctrl.h>
#include <timeapi.h>
#include <windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

using namespace std;

struct PlayerData {
	TCHAR name[MAX_PLAYER_NAME];
	int highScore;
};
extern PlayerData g_player;

struct Player {
	wstring name;
	int score;
};

struct Bullet {
	int x, y;
	int dx, dy;
};
struct EnemyBullet {
	int x, y;
	int dx, dy;
};
struct Enemy {
	int x, y;
	double dx, dy;
	bool alive;
	bool exploding;
	int explosionTimer;
};
extern Enemy enemy;

extern TCHAR g_strLocalPlayerName[MAX_PLAYER_NAME];

extern HINSTANCE                    g_hInst;
extern HWND                       g_hwndMain;
extern int shipX, shipY;
extern int g_score;
extern int g_lives;
extern int enemies_killed;
extern int screenWidth;
extern int screenHeight;
extern std::vector<Bullet>           bullets;
extern std::vector<EnemyBullet>   enemybullets;
extern std::vector<Enemy>            enemies;
extern BOOL                    g_bHostPlayer;
extern HBITMAP                      hShipBmp;
extern HBITMAP                 hExplosionBmp;
extern HBITMAP                     hEnemyBmp;
extern HBITMAP                hBackgroundBmp;
extern HBITMAP                   hBackground;
extern INT                       g_highscore;
extern BOOL                      g_bIsActive;
extern HKEY                       g_hDuelKey;
extern DWORD                   g_dwFrameTime;
extern BOOL                   g_bUseProtocol;
extern BOOL                         g_Paused;
extern BOOL                            alive;
extern BOOL                      g_bReliable;
extern BOOL leftPressed, rightPressed, spacePressed, upPressed, downPressed;
extern BOOL                         g_bAsync;

extern HCURSOR hHand;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK SplashProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

INT loadHighScore(const TCHAR* playerName);

VOID saveHighScore(const TCHAR* name, int score);

INT CALLBACK CompareScores(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

VOID DeleteSelectedPlayer(HWND hList, HWND hDlg);

INT_PTR CALLBACK NameDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK HScoreDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif