#include"tcpServer.h"

extern int nclients;

DWORD WINAPI forClient(LPVOID client_socket)  // Ёта функци€ создаетс€ в отдельном потоке и обсуживает очередного подключившегос€ клиента независимо от остальных
{
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	char buff[20 * 1024];
#define sHELLO "Hello\r\n"


	send(my_sock, sHELLO, sizeof(sHELLO), 0); // отправл€ем клиенту приветствие

	int bytes_recv;
	while ((bytes_recv = recv(my_sock, &buff[0], sizeof(buff), 0)) && bytes_recv != SOCKET_ERROR)
	{
		//send(my_sock, &buff[0], bytes_recv, 0);

		std::cout << std::endl << buff;
	}

	// если мы здесь, то произошел выход из цикла по причине возращени€ функцией recv ошибки Ц соединение с клиентом разорвано

	nclients--; // уменьшаем счетчик активных клиентов
	printf("-disconnect\n"); PRINTNUSERS

		closesocket(my_sock);
	return 0;
}

