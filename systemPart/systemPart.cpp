#include"systemPart.h"


std::wstring SysInfo::strToWstr(std::string str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;    //???
	std::wstring wstr = converter.from_bytes(str);
	return wstr;
}

std::wstring SysInfo::ReadHostFile() 
{
	const TCHAR szFileName[] = "C:\\windows\\system32\\drivers\\etc\\hosts";
	std::string str = "";
	DWORD dwCount;
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		 std::cout << "CreateFile() Error:  " << GetLastError() << std::endl; //_getch();
	}

	LARGE_INTEGER size_ulrg;
	int size = -1;//__int64 size = -1;
	if (!GetFileSizeEx(hFile, &size_ulrg))
	{
		CloseHandle(hFile);
		std::cout << "GetFileSizeEx() Error:  " << GetLastError() << std::endl;// _getch();
	}
	size = size_ulrg.QuadPart;

	if (size > 0)
	{

		char * mem = new char[size];
		ReadFile(hFile, mem, size, &dwCount, NULL);
		if (dwCount != size)
		{
			CloseHandle(hFile);
			std::cout << "ReadFile()_2 Error:  " << GetLastError() << std::endl; //_getch();
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
	GetComputerName(buffer, &size);
	stream << buffer << "              computerName" << std::endl;

	GetUserName(buffer, &size);
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

			switch (GetDriveType(dd))
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
			//ULARGE_INTEGER FreeBytesAvailable;
			//ULARGE_INTEGER TotalNumberOfFreeBytes;
			ULARGE_INTEGER TotalNumberOfBytes;

			BOOL GetDiskFreeSpaceFlag = GetDiskFreeSpaceEx(
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
	SysInfo SI;
	std::wstring sysInfo = SI.SystemInfoToString();              // std::wstring sysInfo = SystemInfoToString() + ReadHostFile();//can send only part....   !!!
	std::wstring hostFile = SI.ReadHostFile();

	std::wcout << sysInfo << std::endl << hostFile << std::endl;



	LPWSTR strRequest = new WCHAR[BUFFER_SIZE + 1];
	memset(strRequest, 0, (BUFFER_SIZE + 1) * sizeof(WCHAR));
	DWORD sendBytesRead = 0;
	DWORD sendBytesWrited = 0;
	BOOL isSuccess = FALSE;

	HANDLE pipe = (HANDLE)hPipe;

	std::wcout << L"Instance created and waiting for messages" << std::endl;

	while (TRUE)
	{
		isSuccess = ReadFile(
			pipe,
			strRequest,
			BUFFER_SIZE * sizeof(WCHAR),
			&sendBytesRead,
			NULL);
		if (!isSuccess)
		{
			std::wcout << L"Error of reading client message or client disconnected" << std::endl;
			break;
		}
		else
			std::wcout << strRequest << std::endl;
		///////////////
		isSuccess = WriteFile(
			hPipe,
			sysInfo.c_str(),
			(sysInfo.size() + 1) * sizeof(WCHAR),
			&sendBytesWrited,
			NULL);

		if (!isSuccess)
		{
			std::wcout << L"Unable to send sysIn" << std::endl;
			EXIT_FAILURE;
		}
		else
			std::wcout << L"sysInfo was send" << std::endl;
		/////////////
		isSuccess = WriteFile(
			hPipe,
			hostFile.c_str(),
			(hostFile.size() + 1) * sizeof(WCHAR),
			&sendBytesWrited,
			NULL);

		if (!isSuccess)
		{
			std::wcout << L"Unable to send hostFile" << std::endl;
			EXIT_FAILURE;
		}
		else
			std::wcout << L"hostFile was send" << std::endl;
	}

	if (strRequest)
		delete[] strRequest;

	ExitThread(0);
	CloseHandle(pipe);
}

