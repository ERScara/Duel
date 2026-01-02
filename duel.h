#pragma once
#ifndef DUEL_INCLUDED
#define DUEL_INCLUDED

#if defined(__BORLANDC__) && !defined(__cplusplus)
#define NONAMELESSSNION
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "resource.h"
#include <wtypes.h>
#include <tchar.h>


#define EXITCODE_FORWARD      1
#define EXITCODE_BACKUP       2
#define EXITCODE_QUIT         3
#define EXITCODE_ERROR        4
#define EXITCODE_LOBBYCONNECT 5

#define UM_LAUNCH        WM_USER
#define UM_ABORT         WM_USER+1
#define UM_RESTARTTIMER  WM_USER+2

enum {PS_SPLASH, PS_ACTIVE, PS_RESET };

#define MAX_SCREEN_X      639
#define MAX_SCREEN_Y      479
#define MAX_PLAYER_NAME    14
#define MAX_SESSION_NAME  256
#define MAX_SPNAME         50
#define MAX_CLASSNANE      50
#define MAX_WINDOWTITLE    50
#define MAX_ERRORMSG      256
#define MAX_FONTNAME       50
#define MAX_HELPMSG       512

#define RECEIVE_TIMER_ID    1
#define RECEiVE_TIMEOUT  1000

#define ENUM_TIMER_ID       2
#define ENUM_TIMEOUT     2000

#define MAX_DEFWIN_X      640
#define MAX_DEFWIN_Y      480

#define CX_BITMAP          25
#define CY_BITMAP          25
#define NUM_BITMAPS         2

#define DUEL_KEY (TEXT("C:\\Users\\Esteban\\source\\repos\\NewProjects\\Duel"))

VOID ShowError(int err);
VOID UpdateTitle();

int randInt(int low, int high);
double randDouble(double low, double high);
#define TRACE dtrace 
void dtrace(TCHAR* strFormat, ...);

#endif