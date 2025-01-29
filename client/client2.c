#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("Error al crear el socket");
    }

    // Configurar dirección del servidor
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    char ip_servidor[20];
    printf("Ingrese la dirección IP del servidor: ");
    scanf("%s", ip_servidor);

    if (inet_pton(AF_INET, ip_servidor, &server_address.sin_addr) <= 0) {
        error("Dirección inválida/No soportada");
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        error("Error al conectar al servidor");
    }

    // Solicitar archivo
    printf("Ingrese el nombre del archivo a solicitar: ");
    scanf("%s", buffer);
    send(sock, buffer, strlen(buffer), 0);

    // Recibir el archivo
    FILE *file = fopen("archivo_recibido", "wb");
    if (file == NULL) {
        error("Error al crear el archivo recibido");
    }

    printf("Recibiendo archivo...\n");
    size_t bytes;
    while ((bytes = read(sock, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, bytes, file);
    }

    printf("Archivo recibido con éxito.\n");

    fclose(file);
    close(sock);
    return 0;
}
