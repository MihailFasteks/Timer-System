#define WM_ICON WM_APP
#define ID_TRAYICON WM_USER
#include "hw-22.h"
#include "framework.h"
#define _CRT_SECURE_NO_WARNINGS

using namespace std;


CWaitableTimerDlg* CWaitableTimerDlg::ptr = NULL;

struct ThreadParams {
	int index;
	HWND hListState;
};
ThreadParams* paramList = new ThreadParams;

CWaitableTimerDlg::CWaitableTimerDlg(void)
{
	ptr = this;
	pNID = new NOTIFYICONDATA;
}

CWaitableTimerDlg::~CWaitableTimerDlg(void)
{
	delete pNID;
}

void CWaitableTimerDlg::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

UINT editIds[] = { IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4 };

BOOL CWaitableTimerDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{

	//list
	hListTasks = GetDlgItem(hwnd, IDC_LIST1);
	hListDate = GetDlgItem(hwnd, IDC_LIST3);
	hListState = GetDlgItem(hwnd, IDC_LIST2);

	//buttons
	hBAddTask = GetDlgItem(hwnd, IDC_BUTTON1);
	hBEditTask = GetDlgItem(hwnd, IDC_BUTTON2);
	hBDeleteTask = GetDlgItem(hwnd, IDC_BUTTON3);
	hBFinishTask = GetDlgItem(hwnd, IDC_BUTTON4);

	//edits
	hEditTask = GetDlgItem(hwnd, IDC_EDIT1);
	hEditDateH = GetDlgItem(hwnd, IDC_EDIT2);
	hEditDateM = GetDlgItem(hwnd, IDC_EDIT3);
	hEditDateS = GetDlgItem(hwnd, IDC_EDIT4);

	//spins
	hSpin1 = GetDlgItem(hwnd, IDC_SPIN1);
	hSpin2 = GetDlgItem(hwnd, IDC_SPIN2);
	hSpin3 = GetDlgItem(hwnd, IDC_SPIN3);

	hDialog = hwnd;

	// Установим необходимый диапазон для счётчиков
	SendMessage(hSpin1, UDM_SETRANGE32, 0, 23);
	SendMessage(hSpin2, UDM_SETRANGE32, 0, 59);
	SendMessage(hSpin3, UDM_SETRANGE32, 0, 59);

	// Получим дескриптор экземпляра приложения
	HINSTANCE hInst = GetModuleHandle(NULL);

	memset(pNID, 0, sizeof(NOTIFYICONDATA)); //Обнуление структуры
	pNID->cbSize = sizeof(NOTIFYICONDATA); //размер структуры
	pNID->hIcon = hIcon; //загружаем пользовательскую иконку
	pNID->hWnd = hwnd; //дескриптор окна, которое будет получать уведомляющие сообщения,
	// ассоциированные с иконкой в трэе.	
	lstrcpy(pNID->szTip, TEXT("Будильник")); // Подсказка

	pNID->uCallbackMessage = WM_ICON;

	pNID->uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE | NIF_INFO;

	lstrcpy(pNID->szInfo, TEXT("Приложение демонстрирует работу таймера синхронизации"));
	lstrcpy(pNID->szInfoTitle, TEXT("Будильник!"));
	pNID->uID = ID_TRAYICON; // предопределённый идентификатор иконки
	return TRUE;
}

DWORD WINAPI Thread(LPVOID lp)
{
	CWaitableTimerDlg* p = (CWaitableTimerDlg*)lp;
	int length = SendMessage(p->hEditTask, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* pBuffer = new TCHAR[length + 1];
	TCHAR state[] = TEXT("in process");

	GetWindowText(p->hEditTask, pBuffer, length + 1);
	//обнуление
	SetWindowText(p->hEditTask, TEXT(""));


	SendMessage(p->hListTasks, LB_ADDSTRING, 0, LPARAM(pBuffer));
	SendMessage(p->hListState, LB_ADDSTRING, 0, LPARAM(state));

	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	TCHAR buf[10];
	int hours, minutes, seconds;
	GetWindowText(p->hEditDateH, buf, 10);
	hours = _tstoi(buf);
	GetWindowText(p->hEditDateM, buf, 10);
	minutes = _tstoi(buf);
	GetWindowText(p->hEditDateS, buf, 10);
	seconds = _tstoi(buf);

	//обнуление
	SetWindowText(p->hEditDateH, TEXT(""));
	SetWindowText(p->hEditDateM, TEXT(""));
	SetWindowText(p->hEditDateS, TEXT(""));

	TCHAR time[100];
	wsprintf(time, TEXT("%d:%d:%d"), hours, minutes, seconds);
	SendMessage(p->hListDate, LB_ADDSTRING, 0, LPARAM(time));
	SYSTEMTIME st;
	GetLocalTime(&st); // получим текущее локальное время
	if (st.wHour > hours || st.wHour == hours && st.wMinute > minutes ||
		st.wHour == hours && st.wMinute == minutes && st.wSecond > seconds)
	{
		return 0;
	}
	else if (st.wHour < hours && st.wMinute < minutes && st.wSecond < seconds)
	{
		MessageBox(p->hDialog, TEXT("Введено неправильное время!"), TEXT("Добавление задачи"), MB_OK | MB_ICONSTOP);

	}
	st.wHour = hours;
	st.wMinute = minutes;
	st.wSecond = seconds;
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft); // преобразуем структуру SYSTEMTIME в FILETIME
	LocalFileTimeToFileTime(&ft, &ft); // преобразуем местное время в UTC-время 
	// Устанавливаем таймер
	SetWaitableTimer(hTimer, (LARGE_INTEGER*)&ft, 0, NULL, NULL, FALSE);

	// Ожидание перехода таймера в сигнальное состояние
	if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0) {
		MessageBox(0, _T("Time is up!"), 0, 0);

		// Удаление элемента по индексу
		int index = SendMessage(p->hListState, LB_GETCURSEL, 0, 0);
		SendMessage(p->hListState, LB_DELETESTRING, index, 0);
		SendMessage(p->hListState, LB_INSERTSTRING, index, LPARAM(_T("Failed!")));
	}
	//CloseHandle(hTimer); // закрываем дескриптор таймера


	return 0;
}


void CWaitableTimerDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	int check = 0;
	_TCHAR buffName[100] = _T(" ");
	_TCHAR buffTime[100] = _T(" ");

	//добавление
	if (id == IDC_BUTTON1)
	{
		int length = SendMessage(hEditTask, WM_GETTEXTLENGTH, 0, 0);
		TCHAR* pBuffer = new TCHAR[length + 1];

		if (lstrlen(pBuffer))
		{
			int index = SendMessage(hListTasks, LB_FINDSTRINGEXACT, -1, LPARAM(pBuffer));
			if (index == LB_ERR)
			{
				//проверка на пустоту
				for (UINT i = 0; i < sizeof(editIds) / sizeof(editIds[0]); ++i) {
					HWND hEdit = GetDlgItem(hwnd, editIds[i]);

					int textLength = GetWindowTextLength(hEdit);

					if (textLength > 0) {

					}
					else {
						check++;
					}
				}



				if (check == 0)
				{
					HANDLE h;
					h = CreateThread(NULL, 0, Thread, this, 0, NULL);
				}
				else
				{
					MessageBox(hwnd, TEXT("Заполните все поля!"), TEXT("Добавление задачи"), MB_OK | MB_ICONSTOP);
				}

			}
			else
			{
				MessageBox(hwnd, TEXT("Такое дело уже существует!"), TEXT("Добавление задачи"), MB_OK | MB_ICONSTOP);
			}

		}
		delete[] pBuffer;
	}

	//редактирование
	if (id == IDC_BUTTON2)
	{
		int index = SendMessage(hListTasks, LB_GETCURSEL, 0, 0);
		if (index != LB_ERR) // выбран ли элемент списка?
		{
			int length = SendMessage(hEditTask, WM_GETTEXTLENGTH, 0, 0);
			TCHAR* pBuffer = new TCHAR[length + 1];
			GetWindowText(hEditTask, pBuffer, length + 1);
			SendMessage(hListTasks, LB_DELETESTRING, index, 0);
			SendMessage(hListTasks, LB_INSERTSTRING, index, LPARAM(pBuffer));

		}
		else
		{
			MessageBox(hwnd, TEXT("Поле не выбрано!"), TEXT("Изменение дела"), MB_OK | MB_ICONSTOP);
		}
	}

	//удаление
	else if (id == IDC_BUTTON3) {

		int index = SendMessage(hListTasks, LB_GETCURSEL, 0, 0);

		SendMessage(hListTasks, LB_DELETESTRING, index, 0);
		SendMessage(hListDate, LB_DELETESTRING, index, 0);
		SendMessage(hListState, LB_DELETESTRING, index, 0);
	}

	//изменение статуса
	else if (id == IDC_BUTTON4)
	{
		TCHAR state[] = TEXT("finished successful!");
		int index = SendMessage(hListTasks, LB_GETCURSEL, 0, 0);
		SendMessage(hListState, LB_DELETESTRING, index, 0);
		SendMessage(hListState, LB_INSERTSTRING, index, LPARAM(state));
	}


}



BOOL CALLBACK CWaitableTimerDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);

	}
	return FALSE;
}
