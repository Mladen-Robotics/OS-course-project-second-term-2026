#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    // Проверка дали е подаден точно един аргумент към програмата.
    if (argc != 2) {
        // Извеждам пояснение как се използва тя през stderr.
        write(2, "Usage: ./reverse <file>\n", 24);
        return 1;
    }

    // Отваряне на файла за четене и писане
    int fd = open(argv[1], O_RDWR);
    if (fd < 0) { // проверка дали е бил отворен успешно
        perror("reverse");
        return 1;
    }

    // Намиране на размера на файла
    off_t left = 0; // променлива, показваща до къде съм стигнал от лявата граница на файла
    off_t right = lseek(fd, 0, SEEK_END); // променлива, показваща до къде съм стигнал от дясната граница на файла
    if (right < 0) { // при грешка с lseek
        perror("reverse");
        close(fd);
        return 1;
    }

    // Межденен буфер за обръщане на съдържането на файла
    unsigned char buffer[512];

    // Придвижване от двата края на файла към средата
    while (left < right) {
        off_t remaining = right - left; // променлнива, съхраняваща броя останали необърнати байтове във файла
        
        // Максималния брой байтове от едната страна на файла, които да бъдат записани в буфера. 
        size_t chunk = (remaining / 2 > 256) ? 256 : (size_t)(remaining / 2);
        
        // Ако остава само 1 байт в точната среда на файла, няма нужда от размяна
        if (chunk == 0) {
            break;  // програмата напуска зикъла
        }

        // 1. Прочитане на брой битове, посочени от chunk, от лявата страна на файла и запис в buffer.
        if (lseek(fd, left, SEEK_SET) < 0 ||  // Преместване на курсора в началото на файла
            read(fd, buffer, chunk) != (ssize_t)chunk) { 
            perror("reverse"); // При възникнала грешка
            close(fd);
            return 1;
        }

        // 2. Прочитане на брой битове, посочени от chunk, от дясната  страна на файла и запис в buffer.
        if (lseek(fd, right - chunk, SEEK_SET) < 0 || 
            read(fd, buffer + 256, chunk) != (ssize_t)chunk) {
            perror("reverse");
            close(fd);
            return 1;
        }

        // 3. Размяна на байтовете между лявата и дясната част на буфера
        for (size_t i = 0; i < chunk; i++) {
            unsigned char temp = buffer[i]; // Байт от лявото парче
            // Разменяме го с огледалния му байт от дясното парче
            buffer[i] = buffer[2 56 + chunk - 1 - i]; 
            buffer[256 + chunk - 1 - i] = temp;
        }

        // 4. Запис на лявата част на буфера в лявата част на файла 
        if (lseek(fd, left, SEEK_SET) < 0 || 
            write(fd, buffer, chunk) != (ssize_t)chunk) {
            perror("reverse");
            close(fd);
            return 1;
        }

        // 5. Запис на дясната част на буфера в дясната част на файла 
        if (lseek(fd, right - chunk, SEEK_SET) < 0 || 
            write(fd, buffer + 256, chunk) != (ssize_t)chunk) {
            perror("reverse");
            close(fd);
            return 1;
        }

        // Преместване на границите с броя обърнати байтове
        left += chunk;
        right -= chunk;
    }

    close(fd); // затваряне на файловия дескриптор
    return 0;
}
