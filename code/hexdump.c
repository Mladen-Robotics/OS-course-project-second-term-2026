#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    // Проверка за подаден аргумент (път към файл)
    if (argc != 2) {
        // Извеждаме съобщение за грешка директно чрез write, за да избегнем fprintf
        write(2, "Usage: ./hexdump <file>\n", 24);
        return 1;
    }

    // Отваряне на файла само за четене
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("hexdump");
        return 1;
    }

    unsigned char buffer[16];
    char out_buf[80]; // Буфер за един ред (достатъчно голям за hex и ASCII частта)
    const char hex_digits[] = "0123456789ABCDEF";
    ssize_t bytes_read;

    // Четене на парчета от по 16 байта
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        int out_idx = 0;

        // 1. Форматиране на шестнадесетичната част
        for (int i = 0; i < 16; i++) {
            if (i < bytes_read) {
                // Преобразуване на байта в 2 hex знака
                out_buf[out_idx++] = hex_digits[buffer[i] >> 4];
                out_buf[out_idx++] = hex_digits[buffer[i] & 0x0F];
            } else {
                // Ако байтът липсва (край на файла), запълваме с разстояния
                out_buf[out_idx++] = ' ';
                out_buf[out_idx++] = ' ';
            }

            // Добавяне на интервал между всеки байт (без след последния)
            if (i < 15) {
                out_buf[out_idx++] = ' ';
            }
            
            // Добавяне на допълнителен интервал между 8-ми и 9-ти байт
            if (i == 7) {
                out_buf[out_idx++] = ' ';
            }
        }

        // 2. Отстъп от 3 интервала преди ASCII текста
        out_buf[out_idx++] = ' ';
        out_buf[out_idx++] = ' ';
        out_buf[out_idx++] = ' ';

        // 3. Форматиране на ASCII частта
        for (int i = 0; i < bytes_read; i++) {
            // Проверка дали знакът е отпечатваем (между 32 и 127 вкл.)
            if (buffer[i] >= 32 && buffer[i] <= 127) {
                out_buf[out_idx++] = buffer[i];
            } else {
                out_buf[out_idx++] = '.';
            }
        }

        // 4. Нов ред и извеждане на стандартния изход
        out_buf[out_idx++] = '\n';
        
        // write връща броя записани байтове (игнорираме го, но е добра практика да се знае)
        write(1, out_buf, out_idx);
    }

    // Проверка за грешка при четенето
    if (bytes_read < 0) {
        perror("hexdump");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}