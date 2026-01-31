#include <math.h>
#include "gameproc.h"
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


PlayerData g_player;

Enemy enemy = { 200, 50, 3, 0, true };

TCHAR g_strLocalPlayerName[MAX_PLAYER_NAME];

HINSTANCE                    g_hInst;
HWND                      g_hwndMain;
int shipX = 400, shipY = 500;
int g_score = 0;
int g_lives = 5;
int enemies_killed = 100;
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
std::vector<Bullet>           bullets;
std::vector<EnemyBullet>   enemybullets;
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

HCURSOR hHand = LoadCursor(NULL, IDC_HAND);

LRESULT CALLBACK SplashProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HBITMAP hBitMap;

	switch (message) {
	case WM_CREATE:
		hBitMap = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_SPLASH), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		if (!hBitMap) MessageBox(hWnd, L"Image could not be loaded", L"Error", MB_OK);

		BITMAP bm;
		if (hBitMap != NULL) {
			GetObjectW(hBitMap, sizeof(bm), &bm);
		}
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, bm.bmWidth, bm.bmHeight, SWP_NOZORDER);
		SetTimer(hWnd, 1, 5000, NULL);
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HDC hdcMem = CreateCompatibleDC(hdc);
		SelectObject(hdcMem, hBitMap);

		BITMAP bm;
		GetObject(hBitMap, sizeof(bm), &bm);
		BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

		DeleteDC(hdcMem);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_TIMER:
		KillTimer(hWnd, 1);
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		if (hBitMap) DeleteObject(hBitMap);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

INT loadHighScore(const TCHAR* playerName) {
	wifstream file("best.dat");
	if (file.is_open()) {
		wstring name;
		int score = 0;
		wstring target(playerName);
		while (file >> name >> score) {
			if (name == target) {
				lstrcpy(g_strLocalPlayerName, name.c_str());
				g_highscore = score;
				return score;
			}
		}
		lstrcpy(g_strLocalPlayerName, playerName);
		g_highscore = 0;
		return 0;
	}
}

VOID saveHighScore(const TCHAR* name, int score) {
	wifstream infile("best.dat");
	vector<pair<wstring, int>> players;
	wstring pname;
	int pscore;

	while (infile >> pname >> pscore) {
		players.push_back({ pname, pscore });
	}
	infile.close();

	bool found = false;
	for (auto& p : players) {
		if (p.first == name) {
			if (score > p.second) {
				p.second = score;
			}
			found = true;
			break;
		}
	}
	if (!found) {
		players.push_back({ name, score });
	}

	wofstream outfile("best.dat");
	for (auto& p : players) {
		outfile << p.first << L"\t" << p.second << L"\n";
	}
}

INT CALLBACK CompareScores(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	const auto* players = reinterpret_cast<const vector<Player>*>(lParamSort);
	int s1 = (*players)[static_cast<size_t>(lParam1)].score;
	int s2 = (*players)[static_cast<size_t>(lParam2)].score;
	return s2 - s1;
}

VOID DeleteSelectedPlayer(HWND hList, HWND hDlg) {
	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (iSel == -1) {
		MessageBox(hList, L"No player has been selected, please select a player.", L"Delete Player", MB_OK | MB_ICONWARNING);
		return;
	}
	wchar_t name[256];
	ListView_GetItemText(hList, iSel, 0, name, 256);

	if (wcscmp(name, g_strLocalPlayerName) == 0) {
		wstring msg = L"Cannot delete the player " + wstring(name, wcslen(name)) + L" because it's currently active."; MessageBox(hDlg, msg.c_str(), L"Warning", MB_OK | MB_ICONWARNING);
		return;
	}

	int res = MessageBox(hDlg, (wstring(L"Do you wish to delete player ") + wstring(name, wcslen(name)) + L"?\nThe information cannot be recovered.").c_str(), L"Confirm Deletion", MB_YESNO | MB_ICONQUESTION);
	if (res == IDYES) {
		ListView_DeleteItem(hList, iSel);
		wifstream infile(L"best.dat");
		vector<pair<wstring, int>> players;
		wstring pname;
		int score;
		while (infile >> pname >> score) {
			if (pname != name) {
				players.push_back({ pname, score });
			}
		}
		infile.close();
		wofstream outfile(L"best.dat", ios::trunc);
		for (auto& p : players) {
			outfile << p.first << L" " << p.second << L"\n";
		}
		outfile.close();
	}
}

INT_PTR CALLBACK HScoreDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static vector<Player> players;
	switch (message) {
	case WM_INITDIALOG: {
		HWND hList = GetDlgItem(hDlg, IDC_LISTSCORES);
		HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DUEL));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		HCURSOR hHand = LoadCursor(NULL, IDC_HAND);
		SetClassLongPtr(GetDlgItem(hDlg, IDOK), GCLP_HCURSOR, (LONG_PTR)hHand);

		INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_LISTVIEW_CLASSES };
		InitCommonControlsEx(&icc);

		ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		LVCOLUMN lvCol{};
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH;

		lvCol.pszText = (LPWSTR)L"Player";
		lvCol.cx = 90;
		ListView_InsertColumn(hList, 0, &lvCol);

		lvCol.pszText = (LPWSTR)L"Score";
		lvCol.cx = 100;
		ListView_InsertColumn(hList, 1, &lvCol);

		players.clear();
		wifstream infile("best.dat");
		wstring name;
		int score;
		while (infile >> name >> score) {
			players.push_back({ name, score });
		}
		infile.close();

		for (size_t i = 0; i < players.size(); ++i) {
			LVITEM item{};
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = static_cast<int>(i);
			item.pszText = const_cast<LPWSTR>(players[i].name.c_str());
			item.lParam = static_cast<LPARAM>(i);
			ListView_InsertItem(hList, &item);

			wstring scoreStr = to_wstring(players[i].score);
			ListView_SetItemText(hList, static_cast<int>(i), 1, const_cast<LPWSTR>(scoreStr.c_str()));
		}
		ListView_SortItems(hList, CompareScores, reinterpret_cast<LPARAM>(&players));
		return TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_DELETE_PLAYER) {
			HWND hList = GetDlgItem(hDlg, IDC_LISTSCORES);
			DeleteSelectedPlayer(hList, hDlg);
			return TRUE;
		}
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK NameDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG: {
		loadHighScore(g_strLocalPlayerName);
		SetDlgItemText(hDlg, IDC_EDIT1, g_strLocalPlayerName);
		HCURSOR hHand = LoadCursor(NULL, IDC_HAND);
		HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DUEL));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SetClassLongPtr(GetDlgItem(hDlg, IDOK), GCLP_HCURSOR, (LONG_PTR)hHand);
		SetClassLongPtr(GetDlgItem(hDlg, IDCANCEL), GCLP_HCURSOR, (LONG_PTR)hHand);
		return TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			TCHAR buffer[MAX_PLAYER_NAME];
			GetDlgItemText(hDlg, IDC_EDIT1, buffer, MAX_PLAYER_NAME);
			lstrcpy(g_strLocalPlayerName, buffer);
			lstrcpy(g_player.name, g_strLocalPlayerName);
			g_player.highScore = g_highscore;
			saveHighScore(g_strLocalPlayerName, g_highscore);
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}
	return FALSE;
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
