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

int main()
{
	FILE *file = fopen("recv.data", "wb");

	if (!file)
	{
		printf("unable to open file: recv.data\n");
	}

	WSADATA wsaData;

	int ri = 0;
	if ((ri = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		printf("WSAStartup failed\n");
		WSACleanup();
		return -1;
	}

	int m_socket = 0;
	int c_socket = 0;

	int rc = 0;

	struct addrinfo hints;
	struct addrinfo *res = 0;
	struct addrinfo *ptr = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	rc = getaddrinfo(0, "5000", &hints, &res);

	if (rc != 0)
	{
		printf("error getting address info\n");
		return 0;
	}

	ptr = res;

	m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (m_socket < 0)
	{
		printf("error opening socket\n");

		return 0;
	}

	rc = bind(m_socket, ptr->ai_addr, ptr->ai_addrlen);

	if (rc == -1)
	{
		printf("eror binding socket\n");
		return 0;
	}

	rc = listen(m_socket, 2);

	if (rc == -1)
	{
		printf("error listening on socket\n");
		return 0;
	}

	printf("listening for connections...\n");

	c_socket = accept(m_socket, 0, 0);

	if (c_socket == -1)
	{
		printf("error acception client connection\n");
		c_socket = 0;
		return 0;
	}

	printf("client connected\n");


#define BUFFER_SIZE 1024
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	uint64_t total_bytes = 0;

	int bytes = 0;

	int bps_t = 0;
	int bps = 0;

	uint64_t time_l = 0;
	uint64_t time_n = 0;
	float time_t = 0;

	do
	{
		time_n = clock();

		time_t += ((float)(time_n - time_l)) / (float)CLOCKS_PER_SEC;
		time_l = time_n;

		bytes = recv(c_socket, buffer, BUFFER_SIZE, 0);

		if (bytes <= 0)
			break;

		total_bytes += bytes;
		fwrite(buffer, 1, bytes, file);

		memset(buffer, 0, BUFFER_SIZE);

		bps_t += bytes;
		if (time_t >= 1.)
		{
			bps = ((float)bps_t) / time_t;
			time_t = .0;
			bps_t = 0;
		}

		printf("\rreceived: %14" PRIu64 " bytes | %10i B/s", total_bytes, bps);

	} while (bytes > 0);

	printf("\nreceived complete file\n");

	fclose(file);
	close(c_socket);
	close(m_socket);

	return 0;
}