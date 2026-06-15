#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Ако не са подадени команди като аргументи, програмата приключва успешно
    if (argc < 2) {
        return 0;
    }

    // Масив, в който запазваме PID-овете на създадените процеси,
    // за да можем след това да свържем всеки PID с оригиналния му команден низ
    pid_t pids[argc];

    // 1. Стартиране на всички процеси паралелно
    for (int i = 1; i < argc; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            // Грешка при създаването на процес
            perror("prun");
        } else if (pids[i] == 0) {
            // ----- ДЪЩЕРЕН ПРОЦЕС -----

            // Заглушаваме стандартния изход на командата (STDOUT), 
            // за да не се смесва с финалния статус, очакван от тестовете
            int devnull = open("/dev/null", O_WRONLY);
            if (devnull >= 0) {
                dup2(devnull, STDOUT_FILENO);
                close(devnull);
            }

            // Правим копие на низа, тъй като strtok го модифицира на място
            char *cmd_copy = strdup(argv[i]);
            if (!cmd_copy) {
                perror("prun");
                exit(1);
            }

            char *args[256];
            int arg_count = 0;

            // Нарязваме низа на отделни аргументи по интервали и табулации
            char *token = strtok(cmd_copy, " \t\n");
            while (token != NULL && arg_count < 255) {
                args[arg_count++] = token;
                token = strtok(NULL, " \t\n");
            }
            args[arg_count] = NULL; // Задължителен завършващ NULL елемент за execvp

            // Ако командата не е празен низ, я изпълняваме
            if (arg_count > 0) {
                execvp(args[0], args);
                // Ако execvp върне управлението, означава, че е възникнала грешка (напр. командата липсва)
                perror("prun");
            }

            // Прекратяваме дъщерния процес с код 1 при неуспешен execvp
            free(cmd_copy);
            exit(1);
        }
    }

    // 2. Изчакване на приключването на всички дъщерни процеси
    int status;
    pid_t wpid;

    // waitpid(-1, ...) прихваща процесите в реалния ред на тяхното завършване
    while ((wpid = waitpid(-1, &status, 0)) > 0) {
        
        // Намираме индекса на завършилия процес в нашия масив `pids`
        int idx = -1;
        for (int i = 1; i < argc; i++) {
            if (pids[i] == wpid) {
                idx = i;
                break;
            }
        }

        // Ако процесът е намерен, извличаме кода му и го принтираме
        if (idx != -1) {
            int exit_code = -1;

            if (WIFEXITED(status)) {
                // Процесът е завършил нормално
                exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                // Процесът е бил прекратен от системен сигнал
                exit_code = 128 + WTERMSIG(status);
            }

            // Отпечатване на резултата на стандартния изход
            printf("[%d] \"%s\" exited with status %d\n", wpid, argv[idx], exit_code);
        }
    }

    return 0;
}