#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PUERTO 8080
#define TAMANO_BUFFER 1024
#define ARCHIVO_RESERVACIONES "reservaciones.txt"

pthread_mutex_t mutex_archivo;

void *manejar_cliente(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[TAMANO_BUFFER];
    char respuesta[TAMANO_BUFFER];
    int read_size;

    // Leer el mensaje del cliente
    if ((read_size = recv(sock, buffer, TAMANO_BUFFER, 0)) > 0) {
        buffer[read_size] = '\0';
        int id;
        char nombre[100];
        sscanf(buffer, "%d,%99s", &id, nombre);

        pthread_mutex_lock(&mutex_archivo);
        FILE *file = fopen(ARCHIVO_RESERVACIONES, "r");
        if (file == NULL) {
            perror("Error al abrir el archivo");
            exit(1);
        }

        char file_content[2048] = {0}; // Asegurarse de que sea suficientemente grande
        char linea[128];
        int encontrado = 0;
        while (fgets(linea, sizeof(linea), file)) {
            int id_actual;
            char nombre_actual[100];
            sscanf(linea, "%d,%99[^\n]", &id_actual, nombre_actual);
            if (id_actual == id) {
                if (strcmp(nombre_actual, "no asignado") == 0) {
                    sprintf(linea, "%d,%s\n", id, nombre);  // Preparar la nueva línea
                    encontrado = 1;
                    sprintf(respuesta, "Reservación exitosa para %s\n", nombre);
                } else {
                    sprintf(respuesta, "El ID %d ya está reservado\n", id);
                    encontrado = 1;
                }
            }
            strcat(file_content, linea); // Construir el contenido completo
        }

        fclose(file);

        if (!encontrado) {
            sprintf(respuesta, "El ID %d no fue encontrado\n", id);
        } else {
            // Reescribir todo el archivo
            file = fopen(ARCHIVO_RESERVACIONES, "w");
            fputs(file_content, file);
            fclose(file);
        }
        pthread_mutex_unlock(&mutex_archivo);

        send(sock, respuesta, strlen(respuesta), 0);
    }

    if(read_size == 0) {
        puts("Cliente desconectado");
    } else if(read_size == -1) {
        perror("Fallo recv");
    }

    close(sock);
    free(socket_desc);
    return 0;
}


int main() {
    int socket_desc, cliente_sock, c;
    struct sockaddr_in servidor, cliente;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("No se pudo crear el socket");
    }
    puts("Socket creado");

    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(PUERTO);

    if(bind(socket_desc, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        perror("bind falló. Error");
        return 1;
    }
    puts("bind hecho");

    listen(socket_desc, 3);

    puts("Esperando conexiones entrantes...");
    c = sizeof(struct sockaddr_in);

    pthread_mutex_init(&mutex_archivo, NULL);

    while((cliente_sock = accept(socket_desc, (struct sockaddr *)&cliente, (socklen_t*)&c))) {
        puts("Conexión aceptada");

        pthread_t hilo_cliente;
        int *nuevo_sock = malloc(1);
        *nuevo_sock = cliente_sock;

        if(pthread_create(&hilo_cliente, NULL, manejar_cliente, (void*) nuevo_sock) < 0) {
            perror("No se pudo crear el hilo");
            return 1;
        }

        pthread_detach(hilo_cliente);
    }

    if (cliente_sock < 0) {
        perror("Fallo al aceptar");
        return 1;
    }

    pthread_mutex_destroy(&mutex_archivo);

    return 0;
}
