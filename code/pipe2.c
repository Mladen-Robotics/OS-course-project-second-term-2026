#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Помощна функция за парсване на командния низ (като в prun)
void parse_cmd(char *cmd_str, char **args, int max_args) {
    int arg_count = 0;
    char *token = strtok(cmd_str, " \t\n");
    while (token != NULL && arg_count < max_args - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[arg_count] = NULL;
}

int main(int argc, char *argv[]) {
    // Проверка за точно 2 аргумента (името на програмата + 2 команди)
    if (argc != 3) {
        write(2, "Usage: ./pipe2 <cmd1> <cmd2>\n", 29);
        return 1;
    }

    int pipefd[2];
    // Създаване на анонимна тръба
    if (pipe(pipefd) < 0) {
        perror("pipe2");
        return 1;
    }

    // --- СТАРТИРАНЕ НА ПЪРВАТА КОМАНДА (cmd1) ---
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("pipe2");
        return 1;
    } else if (pid1 == 0) {
        // Дъщерен процес 1: Пише в тръбата
        // Насочваме STDOUT към входа за писане на тръбата (pipefd[1])
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("pipe2");
            exit(1);
        }
        // Затваряме вече ненужните за детето копия на дескрипторите
        close(pipefd[0]);
        close(pipefd[1]);

        // Парсваме и изпълняваме
        char *cmd1_copy = strdup(argv[1]);
        char *args1[256];
        parse_cmd(cmd1_copy, args1, 256);

        if (args1[0] != NULL) {
            execvp(args1[0], args1);
            perror("pipe2"); // Извиква се само при грешка в execvp
        }
        free(cmd1_copy);
        exit(1);
    }

    // --- СТАРТИРАНЕ НА ВТОРАТА КОМАНДА (cmd2) ---
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("pipe2");
        return 1;
    } else if (pid2 == 0) {
        // Дъщерен процес 2: Чете от тръбата
        // Насочваме STDIN да взима данни от изхода на тръбата (pipefd[0])
        if (dup2(pipefd[0], STDIN_FILENO) < 0) {
            perror("pipe2");
            exit(1);
        }
        // Затваряме ненужните копия
        close(pipefd[0]);
        close(pipefd[1]);

        // Парсваме и изпълняваме
        char *cmd2_copy = strdup(argv[2]);
        char *args2[256];
        parse_cmd(cmd2_copy, args2, 256);

        if (args2[0] != NULL) {
            execvp(args2[0], args2);
            perror("pipe2");
        }
        free(cmd2_copy);
        exit(1);
    }

    // --- БАЩИН ПРОЦЕС ---
    // КРИТИЧНО: Бащата затваря своите краища на тръбата, за да може cmd2 да получи EOF,
    // когато cmd1 приключи да пише!
    close(pipefd[0]);
    close(pipefd[1]);

    int exit_code = 0;

    // Изчакваме и двата процеса без значение кой ще завърши първи
    pid_t wpid;
    int status;
    while ((wpid = waitpid(-1, &status, 0)) > 0) {
        if (wpid == pid2) {
            // Директно извличаме статуса на дясната команда (cmd2) от 'status'
            if (WIFEXITED(status)) {
                exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                exit_code = 128 + WTERMSIG(status);
            }
        }
    }

    return exit_code;
}