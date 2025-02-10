#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define TAM_MAX 16  // Tamaño para dirección IP
#define MAX_CLIENTS 2

int current_clients = 0;
int server_fd;

// Manejo de errores
void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Captura SIGINT para cerrar el servidor
void handle_sigint(int sig) {
    printf("\nCerrando servidor...\n");
    close(server_fd);
    exit(EXIT_SUCCESS);
}

// Captura SIGCHLD para evitar procesos zombies y actualizar clientes
void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        current_clients--;
    }
}

// Obtener la IP del cliente correctamente
void print_client_ip(struct sockaddr_in *client_addr) {
    printf("Direccion IP del cliente: %s\n", inet_ntoa(client_addr->sin_addr));
}

// Manejo de cliente
void process_child(int client_fd, struct sockaddr_in *client_addr) {
    char buffer[BUFFER_SIZE] = {0};

    print_client_ip(client_addr);

    // Recibir el nombre del archivo
    if (recv(client_fd, buffer, BUFFER_SIZE, 0) <= 0) {
        perror("Error al leer del cliente");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Validación del nombre de archivo
    if (strstr(buffer, "..") || buffer[0] == '/' || strlen(buffer) >= BUFFER_SIZE - 1) {
        perror("Intento de acceso no permitido");
        send(client_fd, "ERROR", 5, 0);
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Archivo solicitado: %s\n", buffer);

    FILE *file = fopen(buffer, "rb");
    if (!file) {
        perror("Error al abrir el archivo");
        send(client_fd, "ERROR", 5, 0);
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    size_t bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_fd, buffer, bytes, 0) < 0) {
            perror("Error al enviar el archivo");
            break;
        }
    }

    fclose(file);
    close(client_fd);
    exit(EXIT_SUCCESS);
}

int main() {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    signal(SIGCHLD, handle_sigchld);
    signal(SIGINT, handle_sigint);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("Error al crear el socket");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error("Error al enlazar el socket");
    }

    if (listen(server_fd, 3) < 0) {
        error("Error al escuchar conexiones");
    }

    while (1) {
        if (current_clients >= MAX_CLIENTS) {
            sleep(1);
            continue;
        }

        int client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0) continue;

        if (fork() == 0) {
            close(server_fd);
            process_child(client_fd, &address);
        }
        close(client_fd);
        current_clients++;
    }

    return 0;
}
