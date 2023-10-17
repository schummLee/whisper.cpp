#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <cstring> // for memset

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[256];
    while (true) {
        memset(buffer, 0, 256);
        ssize_t n = read(client_socket, buffer, 255);
        if (n <= 0) {
            std::cout << "Client disconnected. Thread exiting.\n";
            close(client_socket);
            pthread_exit(nullptr);
        }
        std::cout << "Message: " << buffer << std::endl;
        write(client_socket, buffer, n);
    }
}

void run_server() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket < 0) {
            std::cerr << "Error on accept\n";
            continue;
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, nullptr, handle_client, (void *)&client_socket) < 0) {
            std::cerr << "Error creating thread\n";
        }

        pthread_detach(client_thread);
    }

    close(server_socket);
}
