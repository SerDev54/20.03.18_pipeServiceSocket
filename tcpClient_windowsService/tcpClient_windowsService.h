#pragma once

#include<iostream>
#include<stdio.h>
#include<string.h>
#include<winsock2.h>
#include<windows.h>
#include<sstream>

#include<conio.h>
#pragma warning (disable: 4996) //_CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib") 

#define PORT 777
#define WORK_AS_SERVICE 1
#define SERVICE_NAME  L"MySampleService_7"


#define SERVERADDR "127.0.0.1"


class pipeSock
{
	void SendData(std::string str, SOCKET sock);
	void SendData(const wchar_t* wbuffer, SOCKET sock);

public:
	void SysToServer(SOCKET sock);
};


int ERR(const char* functionName);

int ServiseEssentialWork();

/*
SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
//*/
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);



void WorkAsService(int argc, LPWSTR* argv);