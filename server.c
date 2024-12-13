#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <pthread.h>

#define PORT 9097
#define BUFFER_SIZE 1024

// Íá ÇáãÚÇÏáÉ ÇáÈÓíØÉ ax + b = c
double solve_equation(double a, double b, double c) {
    if (a == 0) {
        return -1; // áÇ íæÌÏ Íá
    }
    return (c - b) / a;
}

// ÚÏÇÏ ÇáÚãáÇÁ ÇáÚÇáãí áÖãÇä ÑŞã İÑíÏ áßá Úãíá
int client_count = 0;
pthread_mutex_t client_count_mutex = PTHREAD_MUTEX_INITIALIZER;

// ÏÇáÉ áÊÓÌíá ÇáãÚÇÏáÇÊ æÇáäÊÇÆÌ İí ãáİ
void log_equation(const char* equation, const char* solution) {
    FILE* log_file = fopen("equation_log.txt", "a");
    if (log_file == NULL) {
        perror("Unable to open log file");
        return;
    }
    fprintf(log_file, "Equation: %s, Solution: %s\n", equation, solution);
    fclose(log_file);
}

// ÏÇáÉ ááÊÚÇãá ãÚ ßá ÇÊÕÇá ÌÏíÏ
void* handle_client(void* client_socket_ptr) {
    SOCKET client_fd = *(SOCKET*)client_socket_ptr;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // ÊÚííä ÑŞã İÑíÏ ááÚãíá
    pthread_mutex_lock(&client_count_mutex);
    int client_number = ++client_count;
    pthread_mutex_unlock(&client_count_mutex);

    printf("Client %d connected!\n", client_number);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // ÇÓÊŞÈÇá ÇáãÚÇÏáÉ
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Client %d disconnected\n", client_number);
            closesocket(client_fd);
            return NULL;
        }
        buffer[bytes_received] = '\0'; // ÖãÇä ÇäÊåÇÁ ÇáäÕ
        printf("Received equation from client %d: %s\n", client_number, buffer);

        // ÊÍáíá ÇáãÚÇÏáÉ
        double a, b, c;
        if (sscanf(buffer, "%lfx + %lf = %lf", &a, &b, &c) != 3) {
            printf("Invalid equation format\n");
            closesocket(client_fd);
            return NULL;
        }

        // Íá ÇáãÚÇÏáÉ
        double solution = solve_equation(a, b, c);
        char result[BUFFER_SIZE];

        if (solution == -1) {
            sprintf(result, "No solution (a = 0)");
        }
        else {
            sprintf(result, "x = %.2f", solution);
        }

        // ÊÓÌíá ÇáãÚÇÏáÉ æÇáäÊíÌÉ
        log_equation(buffer, result);

        // ÅÑÓÇá ÇáäÊíÌÉ ááÚãíá
        send(client_fd, result, strlen(result), 0);
        printf("Result sent to client %d: %s\n", client_number, result);
    }

    // ÅÛáÇŞ ÇÊÕÇá ÇáÚãíá
    closesocket(client_fd);
    return NULL;
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // ÊåíÆÉ ãßÊÈÉ Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(EXIT_FAILURE);
    }

    // ÅäÔÇÁ ÓæßíÊ ÇáÎÇÏã
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        perror("Socket creation failed");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // ÑÈØ ÇáÓæßíÊ ÈÇáÎÇÏã
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("Bind failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // ÇÓÊãÇÚ ááÇÊÕÇáÇÊ
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        perror("Listen failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd == INVALID_SOCKET) {
            perror("Accept failed");
            continue;
        }

        // ÅäÔÇÁ ÎíØ ÌÏíÏ ááÊÚÇãá ãÚ ÇáÚãíá
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void*)&client_fd);
        pthread_detach(thread_id);
    }

    // ÊäÙíİ ãßÊÈÉ Winsock
    closesocket(server_fd);
    WSACleanup();

    return 0;
}
