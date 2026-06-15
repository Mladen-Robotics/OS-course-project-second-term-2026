#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(2, "Usage: ./timeserver <port>\n", 27);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        write(2, "timeserver: Invalid port\n", 25);
        return 1;
    }

    // 1. Създаване на TCP сокет
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("timeserver");
        return 1;
    }

    // 2. Активиране на SO_REUSEADDR опцията
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("timeserver");
        close(server_fd);
        return 1;
    }

    // 3. Структура за адрес (0.0.0.0:<port>)
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Свързва се към 0.0.0.0
    address.sin_port = htons(port);

    // 4. Bind (свързване) на сокета към адреса
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("timeserver");
        close(server_fd);
        return 1;
    }

    // 5. Listen за входящи връзки (опашка от 1)
    if (listen(server_fd, 1) < 0) {
        perror("timeserver");
        close(server_fd);
        return 1;
    }

    // 6. Приемане на една входяща връзка
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("timeserver");
        close(server_fd);
        return 1;
    }

    // 7. Вземане и форматиране на текущото местно време
    time_t raw_time = time(NULL);
    struct tm *time_info = localtime(&raw_time);
    
    char time_buffer[64];
    // Формат: YYYY-MM-DD HH:MM:SS\n
    size_t len = strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S\n", time_info);

    // 8. Изпращане на времето към клиента
    if (len > 0) {
        write(client_fd, time_buffer, len);
    }

    // 9. Затваряне на връзките и изход
    close(client_fd);
    close(server_fd);
    return 0;
}