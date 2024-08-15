#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> // Include ini diperlukan untuk inet_ntoa

#define PORT 6000
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Membuat socket TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Mengatur opsi socket untuk reusable address dan port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Mengikat socket ke alamat dan port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Mendengarkan semua antarmuka
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Mendengarkan koneksi
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // Menerima koneksi dari klien
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::cout << "Connection from " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;

        // Menerima data dari klien
        while (true) {
            ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                break;
            }
            // Menampilkan data yang diterima
            std::cout << "Received: " << std::string(buffer, bytes_read) << std::endl;
        }

        // Menutup koneksi dengan klien
        close(new_socket);
        std::cout << "Connection closed with " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;
    }

    return 0;
}
