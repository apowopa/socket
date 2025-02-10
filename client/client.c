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
    scanf("%19s", ip_servidor); // Evita buffer overflow

    if (inet_pton(AF_INET, ip_servidor, &server_address.sin_addr) <= 0) {
        error("Dirección inválida/No soportada");
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        error("Error al conectar al servidor");
    }

    // Solicitar archivo
    printf("Ingrese el nombre del archivo a solicitar: ");
    scanf("%1023s", buffer); // Evita buffer overflow

    if (send(sock, buffer, strlen(buffer), 0) == -1) {
        error("Error al enviar la solicitud");
    }

    // Cerrar el canal de escritura
    shutdown(sock, SHUT_WR);

    // Preguntar por el nombre del archivo donde se guardará
    char nombre_guardado[BUFFER_SIZE];
    printf("Ingrese el nombre para guardar el archivo: ");
    scanf("%1023s", nombre_guardado);

    FILE *file = fopen(nombre_guardado, "wb");
    if (file == NULL) {
        error("Error al crear el archivo recibido");
    }

    printf("Recibiendo archivo...\n");
    size_t bytes;
    while ((bytes = read(sock, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, bytes, file);
        memset(buffer, 0, BUFFER_SIZE); // Evita datos residuales en el buffer
    }

    printf("Archivo recibido con éxito.\n");

    fflush(file);
    fclose(file);
    close(sock);
    return 0;
}
