#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")



int main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;

	// Validate the parameters
	if (argc != 3) {
		printf("usage: %s server-name file\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], "5000", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		if (ptr->ai_family == AF_INET6)
		{
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				return 1;
			}

			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	printf("connected to server\n");

	FILE *file = fopen(argv[2], "rb");

	if (!file)
	{
		printf("error opening file\n");
		return 0;
	}

	printf("opened file: %s\n", argv[2]);

	uint64_t total_bytes = 0;
	unsigned int bytes = 0;

	uint8_t buffer[1024];
	memset(buffer, 0, 1024);
	uint64_t read_bytes = 0;

	int bps_t = 0;
	int bps = 0;

	uint64_t time_l = 0;
	uint64_t time_n = 0;
	float time_t = 0;

	while ((read_bytes = fread(buffer, 1, 1024, file)) > 0)
	{
		time_n = clock();

		time_t += ((float)(time_n - time_l))/(float)CLOCKS_PER_SEC;
		time_l = time_n;

		bytes = send(ConnectSocket, (const char*)buffer, read_bytes, 0);

		if (bytes == SOCKET_ERROR)
		{
			printf("failed to send with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}

		total_bytes += bytes;

		bps_t += bytes;
		if (time_t >= 1.)
		{
			bps = ((float)bps_t)/time_t;
			time_t = .0;
			bps_t = 0;
		}

		printf("\rsent: %14" PRIu64 " bytes | %10i B/s", total_bytes, bps);
		memset(buffer, 0, 1024);
	}

	printf("\nfinished sending file\n");


	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}