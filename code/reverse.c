#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    // Проверка за правилен брой аргументи
    if (argc != 2) {
        // Използваме write вместо fprintf, за да спазим забраните за stdio.h
        write(2, "Usage: ./reverse <file>\n", 24);
        return 1;
    }

    // Отваряне на файла за четене и писане
    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("reverse");
        return 1;
    }

    // Намиране на размера на файла
    off_t left = 0;
    off_t right = lseek(fd, 0, SEEK_END);
    if (right < 0) {
        perror("reverse");
        close(fd);
        return 1;
    }

    // Буфер с фиксиран размер от точно 512 байта
    unsigned char buffer[512];

    // Движим се от двата края към средата
    while (left < right) {
        off_t remaining = right - left;
        
        // Максималното парче за четене от едната страна е 256 байта (половината буфер)
        size_t chunk = (remaining / 2 > 256) ? 256 : (size_t)(remaining / 2);
        
        // Ако остава само 1 байт в точната среда на файла, няма нужда от размяна
        if (chunk == 0) {
            break; 
        }

        // 1. Четене на лявото парче в първата половина на буфера (индекси 0..255)
        if (lseek(fd, left, SEEK_SET) < 0 || 
            read(fd, buffer, chunk) != (ssize_t)chunk) {
            perror("reverse");
            close(fd);
            return 1;
        }

        // 2. Четене на дясното парче във втората половина на буфера (индекси 256..511)
        if (lseek(fd, right - chunk, SEEK_SET) < 0 || 
            read(fd, buffer + 256, chunk) != (ssize_t)chunk) {
            perror("reverse");
            close(fd);
            return 1;
        }

        // 3. Обръщане на байтовете и едновременната им размяна между ляво и дясно
        for (size_t i = 0; i < chunk; i++) {
            unsigned char temp = buffer[i]; // Байт от лявото парче
            // Разменяме го с огледалния му байт от дясното парче
            buffer[i] = buffer[256 + chunk - 1 - i]; 
            buffer[256 + chunk - 1 - i] = temp;
        }

        // 4. Запис на обърнатото дясно парче на лявата позиция
        if (lseek(fd, left, SEEK_SET) < 0 || 
            write(fd, buffer, chunk) != (ssize_t)chunk) {
            perror("reverse");
            close(fd);
            return 1;
        }

        // 5. Запис на обърнатото ляво парче на дясната позиция
        if (lseek(fd, right - chunk, SEEK_SET) < 0 || 
            write(fd, buffer + 256, chunk) != (ssize_t)chunk) {
            perror("reverse");
            close(fd);
            return 1;
        }

        // Стесняване на прозореца
        left += chunk;
        right -= chunk;
    }

    close(fd);
    return 0;
}