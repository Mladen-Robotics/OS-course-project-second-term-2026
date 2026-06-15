#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Глобални флагове, декларирани като volatile sig_atomic_t съгласно условието
volatile sig_atomic_t shared_counter = 1;
volatile sig_atomic_t should_exit = 0;
volatile sig_atomic_t status_requested = 0;

// Обработчик за SIGINT и SIGTERM
void handle_exit_signal(int sig) {
    (void)sig; // Предотвратява предупреждение за неизползван аргумент
    should_exit = 1;
}

// Обработчик за SIGUSR1
void handle_status_signal(int sig) {
    (void)sig;
    status_requested = 1;
}

int main(void) {
    struct sigaction sa_exit;
    struct sigaction sa_status;

    // Настройка на обработчика за SIGINT и SIGTERM
    sa_exit.sa_handler = handle_exit_signal;
    sigemptyset(&sa_exit.sa_mask);
    sa_exit.sa_flags = 0; // Без SA_RESTART, за да може sleep() да бъде прекъсван веднага

    if (sigaction(SIGINT, &sa_exit, NULL) < 0 || sigaction(SIGTERM, &sa_exit, NULL) < 0) {
        perror("worker: sigaction exit");
        return 1;
    }

    // Настройка на обработчика за SIGUSR1
    sa_status.sa_handler = handle_status_signal;
    sigemptyset(&sa_status.sa_mask);
    sa_status.sa_flags = 0; // Искаме да прекъсне sleep(), за да отпечатаме статуса веднага

    if (sigaction(SIGUSR1, &sa_status, NULL) < 0) {
        perror("worker: sigaction status");
        return 1;
    }

    // Временна променлива в main, която отразява реално отпечатаните тикове
    int current_tick = 1;

    // Главен безкраен цикъл
    while (!should_exit) {
        // 1. Извеждаме текущия тик
        printf("tick %d\n", current_tick);
        fflush(stdout); // Подсигуряваме, че текстът излиза веднага

        // Обновяваме глобалния споделен брояч, за да може сигналовия обработчик (потенциално) да го знае
        shared_counter = current_tick;

        // 2. Спим точно 1 секунда, като се съобразяваме с прекъсвания от сигнали
        unsigned int remaining = 1;
        while (remaining > 0 && !should_exit) {
            remaining = sleep(remaining);

            // Ако е дошъл сигнал SIGUSR1, обработваме заявката веднага след събуждането
            if (status_requested) {
                status_requested = 0; // Сваляме флага
                printf("status: tick %d\n", current_tick);
                fflush(stdout);
            }
        }

        // Преминаваме към следващия тик
        current_tick++;
    }

    // При излизане от цикъла (след SIGINT или SIGTERM)
    // Тъй като последното "current_tick++" се е изпълнило в края на цикъла, 
    // броят на реално отпечатаните тикове е точно (current_tick - 1)
    printf("shutdown after %d ticks\n", current_tick - 1);
    fflush(stdout);

    return 0;
}