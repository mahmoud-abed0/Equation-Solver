#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 9097
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char equation[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return -1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert address
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    printf("Connected to server. Enter equations (e.g., 2x + 3 = 7). Type 'exit' to quit.\n");

    while (1) {
        printf("Enter equation: ");
        fgets(equation, BUFFER_SIZE, stdin);
        equation[strcspn(equation, "\n")] = '\0'; // Remove newline character

        // If user types 'exit', break out of the loop
        if (strcmp(equation, "exit") == 0) {
            break;
        }

        // Send equation to server
        send(sock, equation, strlen(equation), 0);

        // Receive result
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Result from server: %s\n", buffer);
        }
        else {
            printf("Failed to receive data from server.\n");
        }
    }

    // Clean up
    closesocket(sock);
    WSACleanup();

    return 0;
}
