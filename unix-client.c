
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


int main(int argc, char **argv)
{

    FILE *file = fopen("recv.data", "wb");

    if(!file)
    {
        printf("unable to open file: recv.data\n");
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

    if(rc != 0)
    {
        printf("error getting address info\n");
        return 0;
    }

    ptr = res;

    m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if(m_socket < 0)
    {
        printf("error opening socket\n");

        return 0;
    }

    rc = bind(m_socket, ptr->ai_addr, ptr->ai_addrlen);

    if(rc == -1)
    {
        printf("eror binding socket\n");
        return 0;
    }

    rc = listen(m_socket, 2);

    if(rc == -1)
    {
        printf("error listening on socket\n");
        return 0;
    }

    printf("listening for connections...\n");

    c_socket = accept(m_socket, 0, 0);

    if(c_socket == -1)
    {
        printf("error acception client connection\n");
        c_socket = 0;
        return 0;
    }

    printf("client connected\n");


#define BUFFER_SIZE 1024
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int total_bytes = 0;

    int bytes = 0;
    do
    {
        bytes = recv(c_socket, buffer, BUFFER_SIZE, 0);

        if(bytes <= 0)
            break;

        total_bytes += bytes;
        fwrite(buffer, 1, bytes, file);

        memset(buffer, 0, BUFFER_SIZE);

        printf("\rreceived: %i", total_bytes);

    } while(bytes > 0);

    printf("\nreceived complete file\n");

    fclose(file);
    close(c_socket);
    close(m_socket);

    return 0;
}
