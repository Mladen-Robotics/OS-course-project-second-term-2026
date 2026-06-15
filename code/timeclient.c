#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(2, "Usage: ./timeclient <hostname> <port>\n", 38);
        return 1;
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        write(2, "timeclient: Invalid port\n", 25);
        return 1;
    }

    // 1. Създаване на TCP сокет
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("timeclient");
        return 1;
    }

    // 2. Намиране на хоста (поддържа IP-та като 127.0.0.1 или имена като localhost)
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        write(2, "timeclient: Host not found\n", 27);
        close(sock_fd);
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy(&server_address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    server_address.sin_port = htons(port);

    // 3. Свързване (connect) към сървъра
    if (connect(sock_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("timeclient");
        close(sock_fd);
        return 1;
    }

    // 4. Четене на отговора до затваряне на връзката (EOF)
    char buffer[256];
    ssize_t bytes_read;
    while ((bytes_read = read(sock_fd, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, bytes_read); // Извеждане директно на стандартния изход
    }

    if (bytes_read < 0) {
        perror("timeclient");
        close(sock_fd);
        return 1;
    }

    // 5. Затваряне и изход с код 0
    close(sock_fd);
    return 0;
}