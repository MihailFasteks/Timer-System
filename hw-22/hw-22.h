#pragma once

#include "Resource.h"
#include<windows.h>
#include <windowsX.h>
#include <commctrl.h>
#include <tchar.h>

#pragma comment(lib,"comctl32")
#pragma once

class CWaitableTimerDlg
{
public:
	CWaitableTimerDlg(void);
public:
	~CWaitableTimerDlg(void);
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static CWaitableTimerDlg* ptr;
	void Cls_OnClose(HWND hwnd);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	HWND hBAddTask, hBEditTask, hBDeleteTask, hBFinishTask, hListTasks, hListDate, hListState, hEditTask, hEditDateH, hEditDateM, hEditDateS, hSpin1, hSpin2, hSpin3, hDialog;
	HICON hIcon;
	PNOTIFYICONDATA pNID;
};
