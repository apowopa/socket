#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Función para obtener la IP local de la máquina
void obtener_ip_local(char *ip_buffer) {
    struct ifaddrs *ifaddr, *ifa;
    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            if (strcmp(ifa->ifa_name, "lo") != 0) { // Evita la interfaz de loopback
                strcpy(ip_buffer, inet_ntoa(addr->sin_addr));
                break;
            }
        }
    }
    freeifaddrs(ifaddr);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("Error al crear el socket");
    }

    // Configurar dirección
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Enlazar el socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error("Error al enlazar el socket");
    }

    // Obtener la IP de la máquina
    char ip_local[INET_ADDRSTRLEN];
    obtener_ip_local(ip_local);
    printf("Servidor escuchando en %s:%d...\n", ip_local, PORT);

    // Escuchar conexiones
    if (listen(server_fd, 3) < 0) {
        error("Error al escuchar conexiones");
    }

    // Aceptar conexión
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        error("Error al aceptar la conexión");
    }

    printf("Cliente conectado desde %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // Recibir el nombre del archivo solicitado
    read(client_fd, buffer, BUFFER_SIZE);
    printf("Archivo solicitado: %s\n", buffer);

    // Abrir el archivo
    FILE *file = fopen(buffer, "rb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        send(client_fd, "ERROR", 5, 0);
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Enviar el archivo
    printf("Enviando archivo...\n");
    size_t bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client_fd, buffer, bytes, 0);
    }

    printf("Archivo enviado con éxito.\n");

    fclose(file);
    close(client_fd);
    close(server_fd);
    return 0;
}
