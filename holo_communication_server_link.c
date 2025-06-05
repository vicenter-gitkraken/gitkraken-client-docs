/**
 * @file holo_comm_server.c
 * @brief Server side of a simulated secure holo-communication link.
 *
 * This program demonstrates a basic TCP server that listens for a client,
 * receives "encrypted" messages, "decrypts" them using a simple XOR cipher,
 * and sends an "encrypted" acknowledgment. This illustrates fundamental
 * socket programming concepts in C.
 *
 * Technical Concepts:
 * - TCP/IP Sockets: socket(), bind(), listen(), accept().
 * - Network I/O: send(), recv().
 * - Basic data "encryption" (XOR cipher for simplicity).
 * - Error handling for network operations.
 * - Server-client model.
 *
 * Thematic Elements:
 * - Holo-communication: Simulating a secure data link.
 * - Encryption/Decryption: Basic data protection.
 *
 * Note: The "encryption" used is for illustrative purposes only and is NOT secure.
 * For actual security, use established cryptographic libraries.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For bool

// For socket programming
#ifdef _WIN32
    // Windows-specific headers and WSAStartup/WSACleanup
    #include <winsock2.h>
    #include <ws2tcpip.h> // For inet_ntop, etc.
    #pragma comment(lib, "ws2_32.lib") // Link with Ws2_32.lib
    #define close_socket closesocket
    typedef int socklen_t;
#else
    // POSIX-specific headers
    #include <sys/socket.h>
    #include <netinet/in.h> // For sockaddr_in, INADDR_ANY
    #include <arpa/inet.h>  // For inet_ntop, inet_pton
    #include <unistd.h>     // For close()
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR   -1
    #define close_socket close
#endif

#define SERVER_PORT 8888    // Port the server will listen on
#define BUFFER_SIZE 1024    // Max buffer size for messages
#define XOR_KEY ((unsigned char)0xA5) // Simple XOR key for "encryption"

/**
 * @brief "Encrypts" or "Decrypts" data using a simple XOR cipher.
 * XORing with the same key encrypts and decrypts.
 * @param data The data buffer to process.
 * @param len The length of the data in the buffer.
 */
void xor_cipher(unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        data[i] ^= XOR_KEY;
    }
}

#ifdef _WIN32
/**
 * @brief Initializes Winsock for Windows.
 * @return True if initialization is successful, false otherwise.
 */
bool initialize_winsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        // fprintf(stderr, "WSAStartup failed: %d\n", result);
        return false;
    }
    return true;
}

/**
 * @brief Cleans up Winsock for Windows.
 */
void cleanup_winsock() {
    WSACleanup();
}
#endif

int main() {
    SOCKET listen_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    unsigned char buffer[BUFFER_SIZE];
    int recv_len;

#ifdef _WIN32
    if (!initialize_winsock()) {
        return 1;
    }
#endif

    // 1. Create a socket
    listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        // perror("Error creating listen socket");
#ifdef _WIN32
        // fprintf(stderr, "socket failed with error: %ld\n", WSAGetLastError());
        cleanup_winsock();
#endif
        return 1;
    }
    printf("Listen socket created successfully.\n");

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
    server_addr.sin_port = htons(SERVER_PORT); // Convert port to network byte order

    // 2. Bind the socket to the address and port
    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        // perror("Error binding socket");
#ifdef _WIN32
        // fprintf(stderr, "bind failed with error: %ld\n", WSAGetLastError());
#endif
        close_socket(listen_socket);
#ifdef _WIN32
        cleanup_winsock();
#endif
        return 1;
    }
    printf("Socket bound to port %d.\n", SERVER_PORT);

    // 3. Listen for incoming connections
    if (listen(listen_socket, 5) == SOCKET_ERROR) { // Allow up to 5 pending connections
        // perror("Error listening on socket");
#ifdef _WIN32
        // fprintf(stderr, "listen failed with error: %ld\n", WSAGetLastError());
#endif
        close_socket(listen_socket);
#ifdef _WIN32
        cleanup_winsock();
#endif
        return 1;
    }
    printf("Server listening on port %d...\n", SERVER_PORT);

    // 4. Accept a client connection (blocking call)
    client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket == INVALID_SOCKET) {
        // perror("Error accepting connection");
#ifdef _WIN32
        // fprintf(stderr, "accept failed with error: %ld\n", WSAGetLastError());
#endif
        close_socket(listen_socket);
#ifdef _WIN32
        cleanup_winsock();
#endif
        return 1;
    }

    char client_ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);
    printf("Connection accepted from %s:%d\n", client_ip_str, ntohs(client_addr.sin_port));

    // Close the listening socket as we are handling one connection for this simple server
    // For a server handling multiple clients, this would be kept open, and accept would be in a loop.
    close_socket(listen_socket);
    listen_socket = INVALID_SOCKET; // Mark as closed

    printf("Holo-communication link established. Awaiting transmission...\n");

    // 5. Communication loop: Receive data and send acknowledgments
    while (true) {
        memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        recv_len = recv(client_socket, (char*)buffer, BUFFER_SIZE -1, 0);

        if (recv_len == SOCKET_ERROR) {
            // fprintf(stderr, "recv failed with error: ");
#ifdef _WIN32
            // fprintf(stderr, "%ld\n", WSAGetLastError());
#else
            // perror("");
#endif
            break;
        } else if (recv_len == 0) {
            printf("Client disconnected gracefully.\n");
            break;
        } else if (recv_len > 0) {
            buffer[recv_len] = '\0'; // Null-terminate for safety if it's string data
                                    // Though for encrypted bytes, this isn't strictly necessary
                                    // unless we print it directly before decryption.

            printf("Received %d encrypted bytes: [RAW] ", recv_len);
            for(int i=0; i < recv_len; ++i) printf("%02X ", buffer[i]);
            printf("\n");

            // "Decrypt" the message
            xor_cipher(buffer, recv_len);
            // After XORing, buffer should be null-terminated if it was a string before encryption
            // For safety, ensure it's null-terminated again IF we treat it as string.
            buffer[recv_len] = '\0'; 


            printf("Decrypted message (%d bytes): \"%s\"\n", recv_len, (char*)buffer);

            // Check for termination command
            if (strcmp((char*)buffer, "CMD_TERMINATE_LINK") == 0) {
                printf("Termination command received. Closing link.\n");
                unsigned char ack_msg[] = "Link termination acknowledged.";
                xor_cipher(ack_msg, strlen((char*)ack_msg));
                send(client_socket, (char*)ack_msg, strlen((char*)ack_msg), 0);
                break;
            }
            
            // Send an "encrypted" acknowledgment
            unsigned char ack_message[50];
            sprintf((char*)ack_message, "ACK: Received '%s'", (char*)buffer);
            size_t ack_len = strlen((char*)ack_message);

            printf("Sending encrypted ACK: [RAW Before Encrypt] %s\n", ack_message);
            xor_cipher(ack_message, ack_len);
            
            if (send(client_socket, (char*)ack_message, ack_len, 0) == SOCKET_ERROR) {
                // perror("Error sending acknowledgment");
#ifdef _WIN32
                // fprintf(stderr, "send failed with error: %ld\n", WSAGetLastError());
#endif
                break;
            }
            printf("Encrypted ACK sent.\n");
        }
    }

    // 6. Close the client socket
    printf("Closing client socket.\n");
    if (client_socket != INVALID_SOCKET) {
        close_socket(client_socket);
    }
    if (listen_socket != INVALID_SOCKET) { // Should be closed already, but for safety
        close_socket(listen_socket);
    }

#ifdef _WIN32
    cleanup_winsock();
#endif

    printf("Holo-communication server shut down.\n");
    return 0;
}

/**
 * To compile (POSIX, e.g., Linux/macOS):
 * gcc holo_comm_server.c -o holo_comm_server
 *
 * To compile (Windows with MinGW or MSVC):
 * gcc holo_comm_server.c -o holo_comm_server.exe -lws2_32  (for MinGW)
 * cl holo_comm_server.c /link Ws2_32.lib (for MSVC Developer Command Prompt)
 *
 * To run:
 * ./holo_comm_server
 *
 * Then run the client to connect to this server.
 *
 * Notes for robustness/enhancements:
 * - Proper multi-client handling (e.g., using select(), poll(), epoll(), or threads/processes per client).
 * - More robust error checking and handling for all socket calls.
 * - A more sophisticated protocol (e.g., message headers with length, type).
 * - Actual strong encryption.
 * - Configuration for port/address.
 * - Logging.
 */
