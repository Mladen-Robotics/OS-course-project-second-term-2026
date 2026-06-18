#include <stdio.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t should_exit = 0;
volatile sig_atomic_t status_requested = 0;

void handle_signal(int sig) { // извиква се при сигнал
    if (sig == SIGUSR1) {
        status_requested = 1;
    } else {
        should_exit = 1;
    }
}

int main(void) {
    struct sigaction sa = { .sa_handler = handle_signal }; 
    sigemptyset(&sa.sa_mask);
    // SA_RESTART не се слага, за да може сигналите да прекъсват sleep() веднага
    sa.sa_flags = 0; 

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    int counter = 1;

    while (!should_exit) {
        printf("tick %d\n", counter);
        fflush(stdout);

        sleep(1);

        if (status_requested) {
            status_requested = 0;
            printf("status: tick %d\n", counter);
            fflush(stdout);
        }

        if (!should_exit) {
            counter++;
        }
    }

    printf("shutdown after %d ticks\n", counter);
    return 0;
}
