#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PUERTO 8080

int main() {
    int sock;
    struct sockaddr_in servidor;
    char mensaje[1024], respuesta[1024];

    // Crear el socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("No se pudo crear el socket");
        return 1;
    }
    printf("Socket creado\n");

    // Configurar los detalles del servidor
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PUERTO);

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        perror("Fallo la conexión. Error");
        return 1;
    }

    printf("Conectado al servidor\n");

    // Obtener datos del usuario
    printf("Ingrese el ID y el Nombre para la reservación (ejemplo: 1,Juan): ");
    scanf("%s", mensaje);

    // Enviar los datos
    if (send(sock, mensaje, strlen(mensaje), 0) < 0) {
        printf("Fallo al enviar los datos");
        return 1;
    }

    // Recibir una respuesta del servidor
    if (recv(sock, respuesta, sizeof(respuesta), 0) < 0) {
        printf("Fallo al recibir la respuesta");
        return 1;
    }

    printf("Respuesta del servidor: %s\n", respuesta);

    // Cerrar el socket
    close(sock);
    return 0;
}
