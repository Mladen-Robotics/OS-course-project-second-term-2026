#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    // Проверка за подаден аргумент (път към файл)
    if (argc != 2) {
        // Извеждане на съобщение  за грешка към stderr(използвам write вместо fprint).
        write(2, "Usage: ./hexdump <file>\n", 24);
        return 1;
    }

    // Отваряне на файла само за четене
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { // проверка дали файла е бил отворен успешно
        perror("hexdump"); 
        return 1;
    }

    unsigned char buffer[16]; // буфер за директен запис на съдържание от файл
    char out_buf[80]; // Буфер за един ред (достатъчно голям за hex и ASCII частта)
    const char hex_digits[] = "0123456789ABCDEF"; // масив с цялата шестнадесетична азбука
    ssize_t bytes_read; // променлива, съхраняваща брой прочетени байтове

    // Четене на парчета от по най-много 16 байта докъто има съдържание във файла.
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        int out_idx = 0; // до къде е запълнен изходния буфер

        // 1. Шеснадесетичен запис на съдържанието на един ред от файла в изходния буфер out_buf
        for (int i = 0; i < 16; i++) {
            if (i < bytes_read) {
                // Преобразуване на байта в 2 hex знака
                out_buf[out_idx++] = hex_digits[buffer[i] >> 4]; // запис на първи шестнадесетичен символ
                out_buf[out_idx++] = hex_digits[buffer[i] & 0x0F]; // запис на втори шестнадесетичен символ
            } else {
                // Ако байтът липсва  или е достигнат край на файла, запълваме с разстояния
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

        // 3. Запис на същия ред в ASCII формат в изходния буфер
        for (int i = 0; i < bytes_read; i++) {
            // Проверка дали знакът може да бъде визуализиран (между 32 и 127 вкл.)
            if (buffer[i] >= 32 && buffer[i] <= 127) {
                out_buf[out_idx++] = buffer[i];
            } else {
                out_buf[out_idx++] = '.'; // Ако не се записва точка
            }
        }

        // 4. Запис на символ за нов ред в края на всеки ред на буфера
        out_buf[out_idx++] = '\n';
        
        // извежда целия изходен буфер в стандартнатния изход
        write(1, out_buf, out_idx);
    }

    // Проверка дали е имало грешки при четенето.
    if (bytes_read < 0) {
        perror("hexdump");
        close(fd);
        return 1;
    }

    close(fd); // Затваряне на файлов дескриптор
    return 0;
}
