#include <Windows.h>
#include "resource.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <queue>

typedef void(*Function)();
std::queue<Function> queueTasks;
CRITICAL_SECTION criticalSection;
CHAR resultStr[1000];
HWND hResult;

VOID OpenFile()
{
	std::ofstream ofs;
	ofs.open("myLogfile.log", std::ios::trunc);
	ofs.close();
}

void WriteMessage(char str[255], int threadID){
	EnterCriticalSection(&criticalSection);
	{
		std::ofstream f("myLogfile.log", std::ios::app);
		f<<"ID: "<< threadID<< ":" << str<< "\n\n";
		f.close();
	}
	LeaveCriticalSection(&criticalSection);
}

static DWORD WINAPI HandleFirstThread(LPVOID lpParam)
{
	while (true){
		Sleep(3000);
		EnterCriticalSection(&criticalSection);
		if (!queueTasks.empty())	{
			strcat_s(resultStr,1000,"Поток 1 вошел в критическую секцию.\n");
			SetWindowText(hResult,resultStr);
			
			Function Task = queueTasks.front();
			queueTasks.pop();
			
			Task();
			LeaveCriticalSection(&criticalSection);
			
			strcat_s(resultStr,1000,"Поток 1 вышел из критической секции.\n");
			SetWindowText(hResult,resultStr);
			WriteMessage("Поток выполнил задачу.", 1);
		} else {
			LeaveCriticalSection(&criticalSection);
		}
	}
	return 0;
}

void Task(){
	srand(time(NULL));
    Sleep(rand()%1000+10000);
	strcat_s(resultStr,1000,"Поток выполнил задачу.\n");
	SetWindowText(hResult,resultStr);
}

static DWORD WINAPI HandleSecondThread(LPVOID lpParam)
{
	while (true){
		Sleep(3000);
		EnterCriticalSection(&criticalSection);
		if (!queueTasks.empty())	{
			
			strcat_s(resultStr,1000,"Поток 2 вошел в критическую секцию.\n");
			SetWindowText(hResult,resultStr);
			
			Function Task = queueTasks.front();
			queueTasks.pop();
			Task();
			
			LeaveCriticalSection(&criticalSection);
			
			strcat_s(resultStr,1000,"Поток 2 вышел из критической секции.\n");
			SetWindowText(hResult,resultStr);
			
			WriteMessage("Поток выполнил задачу.", 2);
		} else {
			LeaveCriticalSection(&criticalSection);
		}
	}
	return 0;
}


VOID CreateThreads()
{
	CreateThread(NULL, 0, HandleFirstThread, 0, 0, NULL);
	CreateThread(NULL, 0, HandleSecondThread, 0, 0, NULL);
}
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{ 
	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_DIALOG1), 0, DlgProc,0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{ 
			HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
			SendMessage(hwnd, WM_SETICON, 1, (LPARAM)hIcon);
			hResult = GetDlgItem(hwnd, IDC_RESULT);
			for (int i = 0; i < 5; i++)	{
				queueTasks.push(Task);
			}
			InitializeCriticalSection(&criticalSection);
			OpenFile();		
			CreateThreads();
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CHECK:
			if(TryEnterCriticalSection(&criticalSection) != 0)
			{ 
				strcat_s(resultStr,1000,"Критическая секция свободна.\n");
				SetWindowText(hResult,resultStr);
				LeaveCriticalSection(&criticalSection);
			}else
			{
				strcat_s(resultStr,1000,"Критическая секция занята.\n");
				SetWindowText(hResult,resultStr);
			}
			break;
		case IDC_EXIT:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return FALSE;
	}
	return FALSE;
}