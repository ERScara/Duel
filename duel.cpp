#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "dsound.lib")
#include "duel.h"
#include <tchar.h>
#include <timeapi.h>
#include <windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <dsound.h>
#include "gameproc.h"


using namespace std;

struct Bullet{
	int x, y;
	int dx, dy;
};
struct EnemyBullet {
	int x, y;
	int dx, dy;
};
struct Enemy {
	int x, y;
	int dx, dy;
	bool alive;
	bool exploding;
	int explosionTimer;
};
Enemy enemy = { 200, 50, 3, 0, true };

TCHAR g_strLocalPlayerName[MAX_PLAYER_NAME];

HINSTANCE                    g_hInst;
HWND                       g_hwndMain;
int shipX = 400,  shipY = 500;
int g_score = 0;
int g_lives = 5;
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
std::vector<Bullet>           bullets;
std::vector<EnemyBullet> enemybullets;
std::vector<Enemy>            enemies;
BOOL                    g_bHostPlayer;
HBITMAP                      hShipBmp;
HBITMAP                 hExplosionBmp;
HBITMAP                     hEnemyBmp;
HBITMAP                hBackgroundBmp;
HBITMAP                   hBackground;
INT                       g_highscore;
BOOL               g_bIsActive = true;
HKEY                g_hDuelKey = NULL;
DWORD                   g_dwFrameTime;
BOOL                   g_bUseProtocol;
BOOL                 g_Paused = false;
BOOL                     alive = true;
BOOL                      g_bReliable;
BOOL leftPressed = false, rightPressed = false, spacePressed = false, upPressed = false, downPressed = false;
BOOL                 g_bAsync;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT loadHighScore() {
	ifstream file("best.dat");
	int g_score = 0;
	if (file.is_open()) {
		file >> g_score;
	}
	return g_score;
}

VOID saveHighScore(int g_score) {
	ofstream file("best.dat");
	if (file.is_open()) {
		file << g_score;
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE pPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	g_hInst = hInstance;

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("GameWindowClass");
	RegisterClass(&wc);

	int winWidth = 900;
	int winHeight = 700;

	int posX = (screenWidth - winWidth) / 2;
	int posY = (screenHeight - winHeight) / 2;

    hShipBmp = (HBITMAP)LoadImage(g_hInst, TEXT("C:\\Users\\Esteban\\source\\repos\\NewProjects\\Duel\\gray_ship.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!hShipBmp) {
		DWORD err = GetLastError();
		TCHAR buf[256];
		wsprintf(buf, TEXT("Ship could not be loaded. Error %lu"), err);
		MessageBox(g_hwndMain, buf, TEXT("Error"), MB_OK);
	}
	hEnemyBmp = (HBITMAP)LoadImage(g_hInst, TEXT("C:\\Users\\Esteban\\source\\repos\\NewProjects\\Duel\\ENE2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!hEnemyBmp) {
		DWORD err = GetLastError();
		TCHAR buf[256];
		wsprintf(buf, TEXT("Ship could not be loaded. Error %lu"), err);
		MessageBox(g_hwndMain, buf, TEXT("Error"), MB_OK);
	}
	hExplosionBmp = (HBITMAP)LoadImage(g_hInst, TEXT("C:\\Users\\Esteban\\source\\repos\\NewProjects\\Duel\\explode.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!hExplosionBmp) {
		DWORD err = GetLastError();
		TCHAR buf[256];
		wsprintf(buf, TEXT("Explosion could not be loaded. Error %lu"), err);
		MessageBox(g_hwndMain, buf, TEXT("Error"), MB_OK);
	}
	hBackgroundBmp = (HBITMAP)LoadImage(g_hInst, TEXT("C:\\Users\\Esteban\\source\\repos\\NewProjects\\Duel\\desert.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!hBackgroundBmp) {
		DWORD err = GetLastError();
		TCHAR buf[256];
		wsprintf(buf, TEXT("Explosion could not be loaded. Error %lu"), err);
		MessageBox(g_hwndMain, buf, TEXT("Error"), MB_OK);
	}

	g_hwndMain = CreateWindowEx(
		0,
		wc.lpszClassName,
		TEXT("Duel"),
		WS_OVERLAPPEDWINDOW,
		posX, posY,
		winWidth, winHeight,
		nullptr, nullptr, hInstance, nullptr
	);

	ShowWindow(g_hwndMain, nCmdShow);
	MSG msg = {};
	while (TRUE) {
		if (g_bIsActive) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				leftPressed = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
				rightPressed = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
				upPressed = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
				downPressed = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;
				spacePressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
				if (alive && !g_Paused) {
					if (leftPressed) shipX -= 5;
					if (rightPressed) shipX += 5;
					if (upPressed)   shipY -= 5;
					if (downPressed) shipY += 5;
					if (g_Paused && spacePressed) {
						if (leftPressed) shipX -= 0;
						if (rightPressed) shipX += 0;
						if (upPressed)   shipY -= 0;
						if (downPressed) shipY += 0;
					}
				}
				/*if (alive && spacePressed) {
					BITMAP bm;
					GetObject(hShipBmp, sizeof(bm), &bm);
					int bulletX = shipX + bm.bmWidth / 11;
					int bulletY = shipY;
					if (upPressed && leftPressed) bullets.push_back({ bulletX, bulletY, -5, 0});
					else if (upPressed && rightPressed) bullets.push_back({ bulletX, bulletY, 0, -5});
					else if (downPressed && rightPressed) bullets.push_back({ bulletX, bulletY, -5, 0});
					else if (downPressed && leftPressed) bullets.push_back({ bulletX, bulletY , 0, -5});
					else if (upPressed) bullets.push_back({ bulletX, bulletY, 0, -5});
					else if (downPressed) bullets.push_back({ bulletX, bulletY, 0, -5});
					else if (rightPressed) bullets.push_back({ bulletX, bulletY, 0, -5});
					else if (leftPressed) bullets.push_back({ bulletX, bulletY, 0, -5});
					else bullets.push_back({ bulletX, bulletY, 0, -5});
				}
				if (alive && spacePressed) {
					BITMAP bm;
					GetObject(hShipBmp, sizeof(bm), &bm);
					int bulletX = shipX + bm.bmWidth / 14;
					int bulletY = shipY;
					if (upPressed && leftPressed) bullets.push_back({ bulletX, bulletY, -5, 0 });
					else if (upPressed && rightPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (downPressed && rightPressed) bullets.push_back({ bulletX, bulletY, -5, 0 });
					else if (downPressed && leftPressed) bullets.push_back({ bulletX, bulletY , 0, -5 });
					else if (upPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (downPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (rightPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (leftPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else bullets.push_back({ bulletX, bulletY, 0, -5 });
				}*/
				if (alive && spacePressed) {
					BITMAP bm;
					GetObject(hShipBmp, sizeof(bm), &bm);
					int bulletX = shipX + bm.bmWidth / 13;
					int bulletY = shipY;
					if (upPressed && leftPressed) bullets.push_back({ bulletX, bulletY, -5, 0 });
					else if (upPressed && rightPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (downPressed && rightPressed) bullets.push_back({ bulletX, bulletY, -5, 0 });
					else if (downPressed && leftPressed) bullets.push_back({ bulletX, bulletY , 0, -5 });
					else if (upPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (downPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (rightPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (leftPressed) bullets.push_back({ bulletX, bulletY, 0, -5 });
					else if (g_Paused && spacePressed) bullets.push_back({ });
					else bullets.push_back({ bulletX, bulletY, 0, -5 });

				}
				for (auto& b : bullets) { b.x += b.dx; b.y += b.dy; }
				if (g_Paused) { for (auto& b : bullets) { b.y -= b.dy; b.x -= b.dx; } }
				bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& b) { return b.y < 0; }), bullets.end());
				InvalidateRect(g_hwndMain, NULL, TRUE);
				Sleep(30);
			}
		}
		else{
			if (!GetMessage(&msg, nullptr, 0, 0)) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return(int)msg.wParam;
}

VOID DrawTransparentBitmap(HDC hdc, HBITMAP hBmp, int x, int y, int width, int height)
{
	if (!hBmp) return;
	BITMAP bm;
	if (GetObject(hBmp, sizeof(bm), &bm) == 0) return;
	HDC hdcMem = CreateCompatibleDC(hdc);
	if (!hdcMem) return;

	HGDIOBJ hOldBmp = SelectObject(hdcMem, hBmp);

	TransparentBlt(hdc, x, y, width, height, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(0, 0, 0));
	SelectObject(hdcMem, hOldBmp);
	DeleteDC(hdcMem);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE: {
		SetTimer(hwnd, 1, 30, NULL);
		SetTimer(hwnd, 2, 2000, NULL);
		HWND hButton = CreateWindow(TEXT("BUTTON"), TEXT("Restart"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 60, 100, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL);
		g_highscore = loadHighScore();
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == 1) {
			alive = true;
			g_score = 0;
			g_lives = 5;
			bullets.clear();
			enemies.clear();
			enemybullets.clear();
		    InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	case WM_TIMER: {
		BITMAP bmShip;
		GetObject(hShipBmp, sizeof(bmShip), &bmShip);
		int shipWidth = bmShip.bmWidth;
		int shipHeight = bmShip.bmHeight;
		if ( wParam == 1) {
			if (g_Paused) return 0;
			if (alive)
			{
				if (leftPressed)  shipX -= 5;
				if (rightPressed) shipX += 5;
				if (upPressed)    shipY -= 5;
				if (downPressed)  shipY += 5;
			}
			for (auto& b : bullets) {
				b.y -= b.dy;
			}
			for (auto& eb : enemybullets) {
				eb.y += eb.dy;
			}
			for (auto& e : enemies) {
				if (e.alive) {
					e.x += e.dx;
					e.y += e.dy;
					if (rand() % 100 == 0) {
						enemybullets.push_back({ e.x + 30, e.y + 64, 0, 5});
						enemybullets.push_back({ e.x + 37, e.y + 64, 0, 5 });
						/*enemybullets.push_back({e.x + 32, e.y + 64, 0, 5});*/
						if(alive) {
						  PlaySound(TEXT("bfire.wav"), NULL, SND_FILENAME | SND_ASYNC);
						}
					}
					if (rand() % 10 == 0) {
						e.dx = (rand() % 2 == 0 ? 2 : -2);
						e.dx = -e.dx;
					}
					if (e.x < 0 || e.x > 1787 - shipWidth) {
						e.dx = -e.dx;
					}
				}
				if (e.exploding) {
					e.explosionTimer--;
					if (e.explosionTimer <= 0) {
						e.exploding = false;
					}
				}
				if (alive) {
					RECT shipRECT = { shipX, shipY, shipX + 64, shipY + 64 };
					RECT enemyRECT = { e.x, e.y, e.x + 64, e.y + 64 };
					RECT Intersect;
					if (IntersectRect(&Intersect, &enemyRECT, &shipRECT)) {
						e.alive = false;
						alive = false;
						leftPressed = rightPressed = upPressed = downPressed = false;
						PlaySound(TEXT("bangbang.wav"), NULL, SND_FILENAME | SND_ASYNC);
						InvalidateRect(hwnd, NULL, TRUE);
					}
				}
				if (shipY > 1300 /*1150*/ /*970*/ - shipHeight) {
					shipY = 1300 /*1150*/ /*970*/ - shipHeight;
				}
				if (shipY < 0) {
					shipY = 0;
				}
				if (shipX < 0) {
					shipX = 0;
				}
				if (shipX > 1787 - shipWidth) {
					shipX = 1787 - shipWidth;
				}
			}
			enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](Enemy& e) { return !e.alive || e.y > 1000; }), enemies.end());
			InvalidateRect(hwnd, NULL, TRUE);
		}
		else if (wParam == 2) {
			if (enemies.size() < 5) {
				Enemy e;
				e.x = rand() % (1300 - 64);
				e.y = 0;
				e.dx = 0;
				e.dy = 2;
				e.alive = true;
				enemies.push_back(e);
				enemies.push_back({ rand() % 400, 0, 0, 2, true });
			}
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == 'R' && g_lives > 0 && !alive) { alive = true; g_lives--; InvalidateRect(hwnd, NULL, TRUE); return 0; };	
		if (wParam == 'P') { g_Paused = !g_Paused; if (g_Paused) { leftPressed = rightPressed = upPressed = downPressed = false; } InvalidateRect(hwnd, NULL, TRUE); };
		if (!alive) return 0;
		if (g_Paused) return 0;
		if (wParam == VK_LEFT) leftPressed = true;
		if (wParam == VK_RIGHT) rightPressed = true;
		if (wParam == VK_UP) upPressed = true;
		if (wParam == VK_DOWN) downPressed = true;
		if (wParam == VK_SPACE) { PlaySound(TEXT("bfire.wav"), NULL, SND_FILENAME | SND_ASYNC); spacePressed = true; };
		return 0;
	case WM_KEYUP:
		if (!alive || g_Paused) return 0;
		if (wParam == VK_LEFT) leftPressed = false;
		if (wParam == VK_RIGHT) rightPressed = false;
		if (wParam == VK_UP) upPressed = false;
		if (wParam == VK_DOWN) downPressed = false;
		if (wParam == VK_SPACE) spacePressed = false;
		return 0;
	case WM_SIZE:
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		RECT rect;
		GetClientRect(hwnd, &rect);
		int width = 1600 - rect.left;
		int height = 1789 - rect.top;
		
		HDC hdcMem = CreateCompatibleDC(hdc);
		SelectObject(hdcMem, hBackgroundBmp);
		BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
		DeleteDC(hdcMem);

		if (hShipBmp && alive) {
			DrawTransparentBitmap(hdc, hShipBmp, shipX, shipY, 64, 64);
		}
		if (!alive && hExplosionBmp) {
			DrawTransparentBitmap(hdc, hExplosionBmp, shipX, shipY, 64, 64);
		}
		
		HBRUSH bulletBrush = CreateSolidBrush(RGB(0, 255, 0));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bulletBrush);
	
		for (auto it = bullets.begin(); it != bullets.end();) {
			Rectangle(hdc, it->x - 2, it->y - 10, it->x + 2, it->y);
			it->x += it->dx;
			it->y += it->dy;
			RECT bulletRECT = { it->x - 2,it->y - 10, it->x + 2, it->y };
			RECT shipRECT = { shipX, shipY, shipX + 64, shipY + 64 };
			RECT Intersect;
			if (IntersectRect(&Intersect, &shipRECT, &bulletRECT)) {
				if (alive) {
					alive = false;
					leftPressed = rightPressed = upPressed = downPressed = false;
					it = bullets.erase(it);
					PlaySound(TEXT("lboom.wav"), NULL, SND_FILENAME | SND_ASYNC);
					continue;
				}
			}
			if (it->y > 1000) {
				it = bullets.erase(it);
			}
			else {
				++it;
			}
		}
		for (auto& e : enemies) {
			RECT enemyRect = { e.x, e.y, e.x + 64, e.y + 64 };
			DrawTransparentBitmap(hdc, hEnemyBmp, e.x, e.y, 64, 64);
			if (e.alive) {
				for (auto it = bullets.begin(); it != bullets.end();) {
					RECT bulletRECT = { it->x - 2, it->y - 10, it->x + 2, it->y };
					RECT enemyRECT = { e.x, e.y, e.x + 64, e.y + 64 };
					RECT Intersect;
					if (IntersectRect(&Intersect, &enemyRECT, &bulletRECT)) {
						e.alive = false;
						g_score += 100;
						
						PlaySound(TEXT("p_bang.wav"), NULL, SND_FILENAME | SND_ASYNC);
						if (g_score % 5000 == 0) {
							g_lives++;
							PlaySound(TEXT("bounce.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NOWAIT);
						}
						it = bullets.erase(it);
					}
					else {
						++it;
					}
				}
			}
			else if (e.exploding) {
				DrawTransparentBitmap(hdc, hExplosionBmp, e.x, e.y, 64, 64);
				e.explosionTimer--;
				if (e.explosionTimer <= 0) { e.exploding = false; }
			}
		}
		if (g_Paused) {  for (auto& b : bullets) { b.y -= b.dy; b.x -= b.dx; } }
	
		HBRUSH enemybulletBrush = CreateSolidBrush(RGB(0, 255, 0));
		HBRUSH oldenemyBrush = (HBRUSH)SelectObject(hdc, enemybulletBrush);
	
		for (const auto& b : enemybullets) {
			Rectangle(hdc, b.x - 2, b.y - 10, b.x + 2, b.y);
		}
		TCHAR scoreBuf[32];
		wsprintf(scoreBuf, TEXT("Points: %d"), g_score);
		SetTextColor(hdc, RGB(0, 255, 0));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 10, 10, scoreBuf, lstrlen(scoreBuf));
		wsprintf(scoreBuf, TEXT("Lives: %d"), g_lives);
		SetTextColor(hdc, RGB(0, 255, 0));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 10, 25, scoreBuf, lstrlen(scoreBuf));
		wsprintf(scoreBuf, TEXT("Best: %d"), g_highscore);
		SetTextColor(hdc, RGB(0, 255, 0));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 10, 40, scoreBuf, lstrlen(scoreBuf));
		if (g_lives == 0 && !alive) {
			HFONT hFont = CreateFont(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
			HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
			SetTextColor(hdc, RGB(255, 0, 0));
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, 550, 400, TEXT("GAME OVER"), lstrlen(TEXT("GAME OVER")));
			if (g_score > g_highscore) {
				g_highscore = g_score;
				saveHighScore(g_highscore);
			}
			if (hdc) {
				PlaySound(TEXT("shield.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
			}
			SelectObject(hdc, oldFont);
			DeleteObject(hFont);
		}
		if (g_score % 5000 == 0 && g_score != 0) {
			HFONT hFont = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));

			HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
			SetTextColor(hdc, RGB(0, 255, 0));
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, 650, 400, TEXT("+1 Extra Life!"), lstrlen(TEXT("+1 Extra Life!")));
			SelectObject(hdc, oldFont);
			DeleteObject(hFont);
		}
		if (g_Paused) {
			HFONT hFont = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
			HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
			SetTextColor(hdc, RGB(0, 255, 255));
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, 650, 400, TEXT("GAME PAUSED"), lstrlen(TEXT("GAME PAUSED")));
			SelectObject(hdc, oldFont);
			DeleteObject(hFont);
		}
		for (auto it = enemybullets.begin(); it != enemybullets.end(); ) {
			if (g_Paused) return 0;
			it->x += it->dx;
			it->y += it->dy;
			RECT bulletRECT = { it->x - 2,it->y - 10, it->x + 2, it->y };
			RECT shipRECT = { shipX, shipY, shipX + 64, shipY + 64 };
			RECT Intersect;
			if (alive && IntersectRect(&Intersect, &shipRECT, &bulletRECT)) {
				alive = false;
				PlaySound(TEXT("lboom.wav"), NULL, SND_FILENAME | SND_ASYNC);
				it = enemybullets.erase(it);
				continue;
			}
			if (it->y > 1000) {
				it = enemybullets.erase(it);
			}
			else {
				++it;
			}
		}
	
		SelectObject(hdc, oldBrush);
		DeleteObject(bulletBrush);

		SelectObject(hdc, oldenemyBrush);
		DeleteObject(enemybulletBrush);

		EndPaint(hwnd, &ps);
		return 0;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


VOID ShowError(int iStrID)
{
	TCHAR strMsg[MAX_ERRORMSG];
	LoadString(g_hInst, iStrID, strMsg, MAX_ERRORMSG);
	MessageBox(g_hwndMain, strMsg, TEXT("Duel Message"), MB_OK);
}

VOID UpdateTitle()
{
	TCHAR strTitle[MAX_WINDOWTITLE] = TEXT("DUEL");

	if (g_bHostPlayer || g_bUseProtocol || g_bReliable || g_bAsync) 
	{
		_tcscat_s(strTitle, TEXT(" - |"));
		if (g_bHostPlayer) _tcscat_s(strTitle, TEXT(" Host |"));
		if (g_bUseProtocol) _tcscat_s(strTitle, TEXT(" Protocol |"));
		if (g_bReliable) _tcscat_s(strTitle, TEXT(" Reliable |"));
		if (g_bAsync) _tcscat_s(strTitle, TEXT(" Async |"));
	}
	SetWindowText(g_hwndMain, strTitle);
}

VOID DoHelp()
{
	TCHAR strHelpMsg[MAX_HELPMSG];
	LoadString(g_hInst, IDS_DUEL_HELP, strHelpMsg, MAX_HELPMSG);
	MessageBox(g_hwndMain, strHelpMsg, TEXT("DUEL"), MB_OK);
}

HRESULT WriteRegKey(HKEY hKey, const TCHAR* strName, TCHAR* strValue, DWORD bufferSize, const TCHAR* strDefault)
{
	DWORD dwType;
	DWORD dwSize = bufferSize * sizeof(TCHAR);
    LONG bResult;

	bResult = RegQueryValueEx(hKey, strName, 0, &dwType, (LPBYTE)strValue, &dwSize);
	if (bResult != ERROR_SUCCESS) {
		_tcscpy_s(strValue, bufferSize, strDefault);
		return S_FALSE;
	}
	return S_OK;
}


HRESULT ReadRegKey(HKEY hKey, TCHAR* strName, TCHAR* strValue, DWORD bufferSize, TCHAR* strDefault) {
	DWORD dwType = 0;
	DWORD dwSize = bufferSize * sizeof(TCHAR);
	LONG bResult = RegQueryValueEx(hKey, strName, nullptr, &dwType, (LPBYTE)strValue, &dwSize);

	if (bResult != ERROR_SUCCESS) {
		_tcscpy_s(strValue, bufferSize, strDefault);
		return HRESULT_FROM_WIN32(bResult);
	}
	return S_OK;
}
