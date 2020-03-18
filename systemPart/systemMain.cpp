//Pipe server. Should interact via pipe with TCP client

#include"systemPart.h"


int main(int argc, char* params[])
{
	BOOL isConnected = FALSE;
	DWORD dwThreadId = 0;
	HANDLE hPipe = NULL;

	std::wstring namePipe(PIPE_BASE_PATH);
	std::wstring tmp = L"testPipe";
	std::list<HANDLE> threads;

	namePipe.append(tmp);

	while (TRUE)
	{
		std::wcout << L"Creating pipe with path" << namePipe << std::endl;
		std::wcout << L"Waiting for clients" << std::endl;

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
			std::wcout << L"Error creating pipe" << namePipe << std::endl;
			return EXIT_FAILURE;
		}

		isConnected = ConnectNamedPipe(hPipe, NULL);
		if (isConnected)
		{
			std::wcout << L"Client connected" << namePipe << std::endl;
			HANDLE hThread = CreateThread(
				NULL,
				NULL,
				InstanceThread,
				(LPVOID)hPipe,
				NULL,
				&dwThreadId);

			if (hThread == INVALID_HANDLE_VALUE)
			{
				std::wcout << L"Error creating thread" << namePipe << std::endl;
			}
			else
			{
				++dwThreadId;
				threads.push_back(hThread);
			}
		}
		else
		{
			std::wcout << L"Error connecting pipe" << namePipe << std::endl;
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
}