
// dllmain.cpp : Defines the entry point for the DLL application.


#define fullDll 1
#define USE_CreateThread 1;

#if fullDll
//////////////////////////h:
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

///////////////////////////////////////////////////

///////////////////////////////////////////////////cpp:
std::wstring SysInfo::strToWstr(std::string str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;    //???
	std::wstring wstr = converter.from_bytes(str);
	return wstr;
}

std::wstring SysInfo::ReadHostFile()
{
	const char* szFileName = "C:\\windows\\system32\\drivers\\etc\\hosts"; //const TCHAR szFileName[] = "C:\\windows\\system32\\drivers\\etc\\hosts";
	std::string str = "";
	DWORD dwCount;
	HANDLE hFile = CreateFileA(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD errorMessageId = ::GetLastError(); MessageBoxA(NULL, "CreateFile() Error", "Info", NULL);//std::cout << "CreateFile() Error:  " << errorMessageId << std::endl; //_getch();
	}

	LARGE_INTEGER size_ulrg;
	int size = -1;//__int64 size = -1;
	if (!GetFileSizeEx(hFile, &size_ulrg))
	{
		CloseHandle(hFile);
		DWORD errorMessageId = ::GetLastError();  MessageBoxA(NULL, "GetFileSizeEx() Error", "Info", NULL);//std::cout << "GetFileSizeEx() Error:  " << errorMessageId << std::endl;// _getch();
	}
	size = size_ulrg.QuadPart;

	if (size > 0)
	{

		char * mem = new char[size];
		ReadFile(hFile, mem, size, &dwCount, NULL);
		if (dwCount != size)
		{
			CloseHandle(hFile);
			DWORD errorMessageId = ::GetLastError(); MessageBoxA(NULL, "GetFileSizeEx() Error", "Info", NULL); //std::cout << "ReadFile()_2 Error:  " << errorMessageId << std::endl; //_getch();
		}
		str = mem;
		delete[] mem;
	}
	CloseHandle(hFile);
	return strToWstr(str);
}


std::wstring SysInfo::SystemInfoToString()
{
	std::ostringstream stream;
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	char buffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(buffer);
	GetComputerNameA(buffer, &size);
	stream << buffer << "              computerName" << std::endl;

	GetUserNameA(buffer, &size);
	stream << buffer << "                 userName" << std::endl;

	int n;
	char dd[4];
	DWORD dr = GetLogicalDrives();
	stream << sysinfo.dwNumberOfProcessors << "                      numberOfProcessors" << std::endl << std::endl;

	stream << "Available disc drives: " << std::endl << std::endl;

	for (int i = 0; i < 26; i++) {
		n = ((dr >> i) & 0x00000001);
		if (n == 1) {
			dd[0] = char(65 + i);
			dd[1] = ':';
			dd[2] = '\\';
			dd[3] = '\0';
			stream << "disk " << dd << std::endl << "\t";

			switch (GetDriveTypeA(dd))
			{
			case DRIVE_UNKNOWN:
				stream << "Uncknown drive";
				break;
			case DRIVE_NO_ROOT_DIR:
				stream << "There's no root directory";
				break;
			case DRIVE_REMOVABLE:
				stream << "removable drive";
				break;
			case DRIVE_FIXED:
				stream << "fixed drive";
				break;
			case DRIVE_REMOTE:
				stream << "remote drive";
				break;
			case DRIVE_CDROM:
				stream << "(CD or DVD)";
				break;
			case DRIVE_RAMDISK:
				stream << "RAM-DISK(emulated disk)";
				break;
			default:;
			}
			stream << std::endl;

			// ***************************************************************************************
			char VolumeNameBuffer[100];
			char FileSystemNameBuffer[100];
			unsigned long VolumeSerialNumber;

			BOOL GetVolumeInformationFlag = GetVolumeInformationA(dd, VolumeNameBuffer, 100, &VolumeSerialNumber,
				NULL, NULL, FileSystemNameBuffer, 100);
			if (GetVolumeInformationFlag != 0) {
				stream << "\tFile System is " << FileSystemNameBuffer << std::endl;
			}
			// ****************************************************************************************
			ULARGE_INTEGER FreeBytesAvailable;
			ULARGE_INTEGER TotalNumberOfBytes;
			ULARGE_INTEGER TotalNumberOfFreeBytes;

			BOOL GetDiskFreeSpaceFlag = GetDiskFreeSpaceExA(
				dd,                                       // directory name
				NULL,                                     // bytes available to caller
				&TotalNumberOfBytes, // bytes on disk
				NULL);                                   // free bytes on disk
			if (GetDiskFreeSpaceFlag != 0)
			{
				stream << "\tTotal Number Of Bytes = " << TotalNumberOfBytes.QuadPart
					<< "( " << TotalNumberOfBytes.QuadPart / (1024 * 1024 * 1024.0)
					<< " Gb )" << std::endl;


			}
		}
	}

	std::string str = stream.str();
	return strToWstr(str);
}

DWORD WINAPI InstanceThread(_In_ LPVOID hPipe)
{

	MessageBoxA(NULL, "Begin of InstanceThread", "Th_sysInfo", NULL);
	SysInfo SI;
	std::wstring sysInfo = SI.SystemInfoToString();              // std::wstring sysInfo = SystemInfoToString() + ReadHostFile();//can send only part....   !!!
	std::wstring hostFile = SI.ReadHostFile();

	//////
	//////
	MessageBoxW(NULL, sysInfo.c_str(), L"Th_sysInfo", NULL);
	MessageBoxW(NULL, hostFile.c_str(), L"hostFile", NULL);
	//////
	//////

	//std::wcout << sysInfo << std::endl << hostFile << std::endl;



	LPWSTR strRequest = new WCHAR[BUFFER_SIZE + 1];
	memset(strRequest, 0, (BUFFER_SIZE + 1) * sizeof(WCHAR));
	DWORD sendBytesRead = 0;
	DWORD sendBytesWrited = 0;
	BOOL isSuccess = FALSE;

	HANDLE pipe = (HANDLE)hPipe;

	////std::wcout << L"Instance created and waiting for messages" << std::endl;

	while (TRUE)
	{

		MessageBoxA(NULL, "Th_(-1) begin of while()", "Th_Info", NULL);



		isSuccess = ReadFile(
			pipe,
			strRequest,
			BUFFER_SIZE * sizeof(WCHAR),
			&sendBytesRead,
			NULL);
		if (!isSuccess)
		{
			MessageBoxA(NULL, "Th_0>>_Error of reading client message or client disconnected", "Th_Info", NULL);
			////std::wcout << L"Error of reading client message or client disconnected" << std::endl;
			break;
		}
		else
			MessageBoxW(NULL, strRequest, L"Th_Info", NULL);

		////std::wcout << strRequest << std::endl;
		///////////////
		isSuccess = WriteFile(
			hPipe,
			sysInfo.c_str(),
			(sysInfo.size() + 1) * sizeof(WCHAR),
			&sendBytesWrited,
			NULL);

		if (!isSuccess)
		{
			MessageBoxA(NULL, "Th_1>> Unable to send sysIn", "Th_Info", NULL);
			//std::wcout << L"Unable to send sysIn" << std::endl;
			EXIT_FAILURE;
		}
		else
			MessageBoxA(NULL, "Th_2>> sysInfo was send", "Th_Info", NULL);

		//std::wcout << L"sysInfo was send" << std::endl;
		/////////////
		isSuccess = WriteFile(
			hPipe,
			hostFile.c_str(),
			(hostFile.size() + 1) * sizeof(WCHAR),
			&sendBytesWrited,
			NULL);

		if (!isSuccess)
		{
			MessageBoxA(NULL, "Th_3>> Unable to send hostFile", "Th_Info", NULL);

			//std::wcout << L"Unable to send hostFile" << std::endl;
			EXIT_FAILURE;
		}
		MessageBoxA(NULL, "Th_4>> hostFile was send", "Th_Info", NULL);

		//else  std::wcout << L"hostFile was send" << std::endl;
	}

	if (strRequest)
		delete[] strRequest;

	ExitThread(0);
	CloseHandle(pipe);
}

#endif
///////////////////////////////////////////////////

int main()//for DLL://BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	//for DLL://if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	//for DLL://{
		MessageBoxA(NULL, "0_>exe>_Hello from 15mar Dll!", "Hello", NULL);  // MessageBoxA(0, "Hello from injected Dll!", 0, 0);
																		 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if fullDll
		BOOL isConnected = FALSE;
		DWORD dwThreadId = 0;
		HANDLE hPipe = NULL;

		std::wstring namePipe(PIPE_BASE_PATH);
		std::wstring tmp = L"testPipe";
		std::list<HANDLE> threads;

		namePipe.append(tmp);

		//MessageBoxA(NULL, "test_1", "Info", NULL);
		while (TRUE)
		{
			//std::wcout << L"Creating pipe with path" << namePipe << std::endl;
			//std::wcout << L"Waiting for clients" << std::endl;
			MessageBoxA(NULL, "1_>exe>_Waiting for clients", "Info", NULL);

			hPipe = CreateNamedPipeW(
				namePipe.c_str(),
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES,
				BUFFER_SIZE,
				BUFFER_SIZE,
				INFINITE,
				NULL);

			if (hPipe == INVALID_HANDLE_VALUE)
			{

				MessageBoxA(NULL, "2_>exe>_Error creating pipe", "Info", NULL);
				return EXIT_FAILURE;
			}

			isConnected = ConnectNamedPipe(hPipe, NULL);
			if (isConnected)
			{
				MessageBoxA(NULL, "3_>exe>_Client connected", "Info", NULL);
#if USE_CreateThread
				HANDLE hThread = CreateThread(
					NULL,
					NULL,
					InstanceThread,
					(LPVOID)hPipe,
					NULL,
					&dwThreadId);
				if (hThread == NULL)//INVALID_HANDLE_VALUE
				{
					MessageBoxA(NULL, "4_>exe>_Error creating thread", "Info", NULL);
				}
				else
				{
					++dwThreadId;
					threads.push_back(hThread);
					MessageBoxA(NULL, "5_>exe>_Thread was successfully created!!!", "Info", NULL);
				}
#else
				InstanceThread(hPipe);
#endif
			}
			else
			{
				MessageBoxA(NULL, "5_>exe>_Error connecting pipe", "Info", NULL);
				return EXIT_FAILURE;
			}
		}
		std::for_each(threads.cbegin(), threads.cend(),
			[](HANDLE h)
		{
			CloseHandle(h);
		});

		CloseHandle(hPipe);
		return EXIT_SUCCESS;

#endif
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//for DLL://}
	return TRUE;
}
