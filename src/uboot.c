#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

char* uboot_file = "/uboot/uboot.dat";

void printHelp() {
    printf("Usage: uboot_tool [COMMAND]\n");
    printf("\n");
    printf("Commands:\n");
    printf(" part_current  - show current partition\n");
    printf(" part_switch   - switch active partition\n");
    printf(" counter       - show boot counter\n");
    printf(" reset_counter - reset boot counter\n");
    printf(" version       - show version of file\n");
}

uint32_t crc32(uint8_t* data, size_t length, uint32_t seed) {
    uint32_t crc = ~seed;
    while (length--) {
        crc ^= (*data++);
        for (unsigned int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xedb88320L;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return ~crc;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }
    char* cmd = argv[1];

    FILE* file = fopen(uboot_file, "rb");
    if (file == NULL) {
        printf("Failed to open uboot file: %s\n", uboot_file);
        return 2;
    }
    
    uint8_t data[1024];
    size_t err = fread(data, 1, sizeof(data), file);
    fclose(file);

    if (err != sizeof(data)) {
        printf("Failed to read full uboot file: %s\n", uboot_file);
        return 2;
    }

    uint32_t stored_crc = ntohl(*(uint32_t*)&data[1020]);
    uint32_t computed_crc = crc32(data, 1020, 0);

    if (stored_crc != computed_crc) {
        fprintf(stderr, "Invalid CRC -> fallback to default\n");

        memset(data, 0, sizeof(data));

        // file version
        data[0] = 1;

        // boot counter
        data[1] = 0;

        // boot partition
        data[2] = 2; // A=3, B=2
    }

    uint8_t save = 0;
    if (strcmp(cmd, "version") == 0) {
        printf("0x%02x\n", data[0]);

    } else if (strcmp(cmd, "part_current") == 0) {
        printf("%d\n", data[2]);

    } else if (strcmp(cmd, "part_switch") == 0) {
        data[2] = (data[2] == 5) ? 6 : 5;
        save = 1;

    } else if (strcmp(cmd, "counter") == 0) {
        printf("%d\n", data[1]);

    } else if (strcmp(cmd, "reset_counter") == 0) {
        data[1] = 0;
        save = 1;

    } else {
        printf("Unknown command\n");
        return 10;
    }

    if (save) {
        computed_crc = crc32(data, 1020, 0);
        *(uint32_t*)&data[1020] = htonl(computed_crc);

        file = fopen(uboot_file, "wb");
        if (file == NULL) {
            printf("Failed to open uboot file: %s\n", uboot_file);
            return 4;
        }
        err = fwrite(data, 1, sizeof(data), file);
        fclose(file);

        if (err != sizeof(data)) {
            printf("Failed to write full uboot file: %s\n", uboot_file);
            return 4;
        }
    }

    return 0;
}