//WorkAsService ==> ServiceMain ==> ServiceWorkerThread ==> ServiseEssentialWork()

#include"tcpClient_windowsService.h"


SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ERR(const char* functionName)
{
	DWORD message = GetLastError();
	std::cout << functionName << "_Error:   " << message << std::endl;
	if (message == 5)std::cout << functionName << "  USE CMD WITH ADMIN PRIVILIGE TO FIX THAT ERROR!!!:   " << message << std::endl;
	else if (message == 1072)std::cout << functionName << "  SERVICE WITH SUCH NAME EXIST AND WAS MARKED FOR DELETION!!!:   " << message << std::endl;
	else if (message == 1073)std::cout << functionName << "  THAT SERVICE ALREADY EXIST!!!:   " << message << std::endl;

	//system("Pause");
	return(-1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pipeSock::SendData(std::string str, SOCKET sock)
{
	const char *ch = strdup(str.c_str());
	send(sock, ch, str.length(), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pipeSock::SendData(const wchar_t* wbuffer, SOCKET sock)
{

	size_t buffer_size;
	wcstombs_s(&buffer_size, NULL, 0, wbuffer, _TRUNCATE);                              // determine the required buffer size


	char *buffer = (char*)malloc(buffer_size);
	wcstombs_s(&buffer_size, buffer, buffer_size, wbuffer, _TRUNCATE);                  // do the actual conversion


	size_t buffer_sent = 0;
	while (buffer_sent < buffer_size)                                                   // send the data
	{
		int sent_size = send(sock, buffer + buffer_sent, buffer_size - buffer_sent, 0);
		buffer_sent += sent_size;
	}
	free(buffer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pipeSock::SysToServer(SOCKET sock)
{
	HANDLE hPipe = NULL;
	DWORD iCountIO = 0;
	DWORD dwMode = 0;
	BOOL isSuccess = FALSE;

	const int strSize = 2048;
	LPWSTR str = new WCHAR[strSize];
	memset(str, 0, (strSize - 1) * 2);

	std::wstring server = L".";  	                                //std::wcout << L"Print server name or .  for localHost: " << std::endl; //std::getline(std::wcin, server);

	std::wstring pipe = L"testPipe";	                            //std::wcout << L"Print name of pipe: " << std::endl;	//std::getline(std::wcin, pipe);

	std::wstring namedPipe = L"\\\\";
	namedPipe.append(server);
	namedPipe.append(L"\\pipe\\");
	namedPipe.append(pipe);
	std::wcout << L"namedPipe= " << namedPipe << std::endl;

	std::wstring message = L"Hello from tcpClient_windowsService  ";             	//std::wcout << L"Print message to server: " << std::endl;	//std::getline(std::wcin, message);


	hPipe = CreateFileW(
		namedPipe.c_str(),
		GENERIC_ALL,
		0, NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		std::wcout << L"Unable to open pipe" << std::endl;
	}

	WaitNamedPipeW(namedPipe.c_str(), INFINITE);

	dwMode = PIPE_READMODE_MESSAGE;

	isSuccess = SetNamedPipeHandleState(
		hPipe,
		&dwMode,
		NULL,
		NULL);

	if (!isSuccess)
	{
		std::wcout << L"Unable to edit mode of pipe" << std::endl;
		EXIT_FAILURE;
	}


	std::wcout << L"Sending message to server" << std::endl;


	isSuccess = WriteFile(
		hPipe,
		message.c_str(),
		(message.size() + 1) * sizeof(WCHAR),
		&iCountIO,
		NULL);

	if (!isSuccess)
	{
		std::wcout << L"Unable to send message" << std::endl;
		EXIT_FAILURE;
	}
	else
		std::wcout << L"message pushed to server" << std::endl;

	//Soooooooooooooooooooooooooooooooooooooooooooooooccccccccccckkkkkkkkk

	int msgCount = 0;
	do
	{
		isSuccess = ReadFile(
			hPipe,
			str,
			(strSize + 1) * sizeof(WCHAR),
			&iCountIO,
			NULL);
		if (isSuccess && iCountIO > 0)
		{

			std::wcout << L"message from systemPart: " << std::endl << str << std::endl;
			SendData(str, sock);

			msgCount++;
		}
	} while (msgCount < 2);//(msgCount < 2);//while (!isSuccess);

	CloseHandle(hPipe);
	delete[] str;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiseEssentialWork()
{
	pipeSock firstConnect;
	char buff[1024];
	printf("winServiceTCPClient\n");

	if (WSAStartup(0x202, (WSADATA *)&buff[0]))// Шаг 1  инициализация библиотеки Winsock
	{
		printf("WSAStart error %d\n", WSAGetLastError());
		_getch();
		return 1;
	}

	SOCKET my_sock;// Шаг 2  создание сокета
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock<0)
	{
		printf("socket() error %d\n", WSAGetLastError());
		_getch();
		return 1;
	}
	// Шаг 3  установка соединения
	// заполнение структуры sockaddr_in – указание адреса и порта сервера
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst;
	// преобразование IP адреса из символьного в сетевой формат
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);


	else
	{
		printf("Invalid address %s\n", SERVERADDR);
		closesocket(my_sock);
		WSACleanup();
		_getch();
		return 1;
	}

	// адрес сервера получен – пытаемся установить соединение
	if (connect(my_sock, (sockaddr *)&dest_addr, sizeof(dest_addr)))
	{
		printf("connect() error %d\n", WSAGetLastError());
		_getch();
		return 1;
	}
	printf("Connection with %s was succesfully established \n", SERVERADDR);
	// Шаг 4  чтение и передача сообщений
	int nsize;
	while ((nsize = recv(my_sock, &buff[0], sizeof(buff) - 1, 0)) != SOCKET_ERROR)
	{

		buff[nsize] = 0;// ставим завершающий ноль в конце строки
		std::cout << buff;
		firstConnect.SysToServer(my_sock);//SendSystemInfo(my_sock);
		Sleep(5000);
	}
	printf("recv() error %d\n", WSAGetLastError());
	closesocket(my_sock);
	WSACleanup();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkAsService(int argc, LPWSTR* argv)
{

	SERVICE_TABLE_ENTRYW ServiceTable[] =
	{
		{ (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONW)ServiceMain },
		{ NULL, NULL }
	};

	if (argc == 2)
	{
		SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);//(NULL, NULL, SC_MANAGER_CREATE_SERVICE)  //(NULL, SERVICES_ACTIVE_DATABASEW, SC_MANAGER_CREATE_SERVICE);
		if (hSCManager == NULL)
			ERR("OpenSCManagerW");

		if (!wcscmp(argv[1], L"-create"))
		{
			std::wcout << L"Creating service..." << std::endl;

			SC_HANDLE handErr = CreateServiceW(
				hSCManager,
				SERVICE_NAME,
				SERVICE_NAME,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_DEMAND_START,
				SERVICE_ERROR_NORMAL,
				argv[0],
				NULL, NULL, NULL, NULL, NULL);

			if (handErr == NULL)
				ERR("CreateServiceW");//return(-1);

			std::wcout << L"Service created!" << std::endl;
		}
		else if (!wcscmp(argv[1], L"-start"))
		{
			std::wcout << L"Starting service..." << std::endl;
			SC_HANDLE service = OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_START);
			if (!StartService(service, 0, nullptr))
				ERR("StartService");
			std::wcout << L"Service started!" << std::endl;
		}
		else if (!wcscmp(argv[1], L"-delete"))
		{
			DeleteService(OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE));
			std::wcout << L"Service deleted!" << std::endl;
		}
		else if (!wcscmp(argv[1], L"-stop"))
		{
			std::wcout << L"Stopping service..." << std::endl;
			DeleteService(OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE));
			SC_HANDLE handErr = CreateServiceW(
				hSCManager,
				SERVICE_NAME,
				SERVICE_NAME,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_DEMAND_START,
				SERVICE_ERROR_NORMAL,
				argv[0],
				NULL, NULL, NULL, NULL, NULL);

			if (handErr == NULL)
				ERR("CreateServiceW");
			std::wcout << L"Service stopped!" << std::endl;

		}
	}

	if (StartServiceCtrlDispatcherW(ServiceTable) == FALSE)
	{
		std::wcout << L"StartServiceCtrlDispatcherW error:    " << GetLastError() << std::endl;
		//system("pause");
		//return GetLastError();
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;

	// Register our service control handler with the SCM
	g_StatusHandle = RegisterServiceCtrlHandlerW(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
		return;

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)	
		OutputDebugStringW(L"My Sample Service: ServiceMain: SetServiceStatus returned error");//send string for debugger for display
	

	/*
	* Perform tasks necessary to start the service here
	*/

	// Create a service stop event to wait on later
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)		
			OutputDebugStringW(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
		
		return;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)	
		OutputDebugStringW(	L"My Sample Service: ServiceMain: SetServiceStatus returned error");
	

	// Start a thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	// Wait until our worker thread exits signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);


	/*
	* Perform any cleanup tasks
	*/

	CloseHandle(g_ServiceStopEvent);

	// Tell the service controller we are stopped
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)	
		OutputDebugStringW(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		* Perform tasks necessary to stop the service here
		*/

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)		
			OutputDebugStringW(L"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error");
		

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		/*
		* Perform main service function here
		*/
		ServiseEssentialWork();//WorkAsService ==> ServiceMain ==> ServiceWorkerThread ==> ServiseEssentialWork()
		Sleep(1000);
	}

	return ERROR_SUCCESS;
}
