#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        write(2, "Usage: ./echoclient <hostname> <port> <message>\n", 48);
        return 1;
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);
    char *message = argv[3];

    if (port <= 0 || port > 65535) {
        write(2, "echoclient: Invalid port\n", 25);
        return 1;
    }

    // 1. Създаване на TCP сокет
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("echoclient");
        return 1;
    }

    // 2. Резолване на хоста
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        write(2, "echoclient: Host not found\n", 27);
        close(sock_fd);
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy(&server_address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    server_address.sin_port = htons(port);

    // 3. Свързване към ехо-сървъра
    if (connect(sock_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("echoclient");
        close(sock_fd);
        return 1;
    }

    // 4. Изпращане на съобщението
    size_t msg_len = strlen(message);
    size_t total_sent = 0;
    while (total_sent < msg_len) {
        ssize_t sent = write(sock_fd, message + total_sent, msg_len - total_sent);
        if (sent <= 0) {
            perror("echoclient: write");
            close(sock_fd);
            return 1;
        }
        total_sent += sent;
    }

    // Затваряме изхода на сокета (shutdown writing), за да покажем на сървъра, 
    // че сме приключили с пращането, но оставяме входа отворен за четене.
    shutdown(sock_fd, SHUT_WR);

    // 5. Четене на ехо отговора и отпечатване на стандартния изход
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(sock_fd, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, bytes_read);
    }
    
    // Добавяме нов ред в края за прегледност на конзолата
    write(1, "\n", 1);

    if (bytes_read < 0) {
        perror("echoclient: read");
        close(sock_fd);
        return 1;
    }

    // 6. Затваряне и изход
    close(sock_fd);
    return 0;
}