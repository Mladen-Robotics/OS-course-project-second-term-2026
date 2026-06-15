#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

// Асинхронно безопасен обработчик за SIGCHLD сигнала
void handle_sigchld(int sig) {
    (void)sig;
    // Прибираме всички умрели дъщерни процеси (зомбита) наведнъж
    // WNOHANG гарантира, че waitpid няма да блокира, ако няма повече завършили деца
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(2, "Usage: ./echoserver <port>\n", 27);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        write(2, "echoserver: Invalid port\n", 25);
        return 1;
    }

    // 1. Настройка на маската и обработчика за SIGCHLD
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Автоматично рестартира accept(), ако бъде прекъснат от сигнал
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("echoserver: sigaction");
        return 1;
    }

    // 2. Създаване на TCP сокет
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("echoserver");
        return 1;
    }

    // 3. Опция SO_REUSEADDR
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("echoserver");
        close(server_fd);
        return 1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    address.sin_port = htons(port);

    // 4. Bind и Listen
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("echoserver");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("echoserver");
        close(server_fd);
        return 1;
    }

    // Безкраен цикъл за приемане на конкурентни клиенти
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            // Ако accept е прекъснат от сигнал, просто опитваме пак
            if (errno == EINTR) {
                continue;
            }
            perror("echoserver: accept");
            continue;
        }

        // Създаване на нов процес за новия клиент
        pid_t pid = fork();
        if (pid < 0) {
            perror("echoserver: fork");
            close(client_fd);
        } else if (pid == 0) {
            // ----- ДЪЩЕРЕН ПРОЦЕС (ОБРАБОТЧИК) -----
            close(server_fd); // Детето няма нужда от слушащия сокет

            char buffer[1024];
            ssize_t bytes_read;

            // Четем на блокове от 1024 байта и пращаме обратно дословно
            while ((bytes_read = read(client_fd, buffer, sizeof(buffer))) > 0) {
                ssize_t bytes_written = 0;
                // Подсигуряваме пълното изпращане на прочетения блок данни
                while (bytes_written < bytes_read) {
                    ssize_t res = write(client_fd, buffer + bytes_written, bytes_read - bytes_written);
                    if (res <= 0) {
                        break; // Грешка при писане или затворен сокет
                    }
                    bytes_written += res;
                }
            }

            close(client_fd);
            exit(0); // Детето приключва работа успешно
        } else {
            // ----- БАЩИН ПРОЦЕС -----
            close(client_fd); // Бащата затваря клиентския сокет и веднага се връща към accept()
        }
    }

    close(server_fd);
    return 0;
}