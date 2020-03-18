#pragma once


#include<Windows.h>
#include<iostream>
#include<string>
#include<list>
#include<algorithm>


#include<sstream>
//#include <locale>
#include <codecvt>

#define PIPE_BASE_PATH L"\\\\.\\pipe\\"
#define BUFFER_SIZE 4096  //???


class SysInfo {
	std::wstring strToWstr(std::string str);

public:
	std::wstring ReadHostFile();	
	std::wstring SystemInfoToString();
};
	
DWORD WINAPI InstanceThread(_In_ LPVOID hPipe);
