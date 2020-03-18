#pragma once

#include <stdio.h>
#include <winsock2.h> // Wincosk2.h should be windows!
#include <windows.h>
#include <iostream>


#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS



#define PRINTNUSERS if (nclients) printf("(%d user online)\n",nclients);else printf("No User online\n");  // makros for printing active user number



DWORD WINAPI forClient(LPVOID client_socket);
