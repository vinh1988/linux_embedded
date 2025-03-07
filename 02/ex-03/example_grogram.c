#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int read_file(const char *filename, int num_bytes) {
    FILE *file = fopen(filename, "r");
    if (file == NULL){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char *buffer = (char *)malloc(num_bytes +1);
    if (buffer == NULL){
        perror("memory allocation failed");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    size_t bytes_read = fread(buffer, 1, num_bytes, file);
    buffer[bytes_read] = '\0';
    printf("Read content: %s\n", buffer);

    free(buffer);
    fclose(file);
}

void write_file(const char *filename, const char *text){
    FILE *file = fopen(filename, "w");
    if (file == NULL){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s", text);
    printf("Successfully written to %s\n", filename);
    fclose(file);
}
int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s filename num-bytes r\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *filename = argv[1];
    int num_bytes = atoi(argv[2]);
    char mode = argv[3][0];

    if (mode == 'r') {
        read_file(filename, num_bytes);
    }else if (mode == 'w') {
        if (argc < 5){
            fprintf(stderr, "Error: Missing text to write\n");
            return EXIT_FAILURE;
        }
        write_file(filename, argv[4]);
    }
    else {
        fprintf(stderr, "Invalid mode. Use 'r' to read.\n");
        return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
}