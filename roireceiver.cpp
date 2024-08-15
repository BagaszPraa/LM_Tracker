#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>

#define BUFFER_SIZE 1024

void processReceivedData(const std::string& data) {
    std::istringstream ss(data);
    std::string token;
    std::vector<int> values;

    // Memisahkan data yang diterima berdasarkan koma
    while (std::getline(ss, token, ',')) {
        try {
            values.push_back(std::stoi(token));  // Mengonversi string ke integer
        } catch (std::exception& e) {
            std::cerr << "Invalid data format: " << e.what() << std::endl;
            return;
        }
    }

    // Mengecek apakah ada 4 nilai yang diterima
    if (values.size() == 4) {
        int x = values[0];
        int y = values[1];
        int width = values[2];
        int height = values[3];

        // Menampilkan nilai yang diterima
        std::cout << "Received: x=" << x << ", y=" << y 
                  << ", width=" << width << ", height=" << height << std::endl;
    } else {
        std::cerr << "Invalid number of values received: " << values.size() << std::endl;
    }
}

void startServer(const std::string& ipAddress, int port) {
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
    address.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Mendengarkan koneksi
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on " << ipAddress << ":" << port << std::endl;

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

            // Mengubah buffer menjadi string dan memproses data
            std::string received_data(buffer, bytes_read);
            processReceivedData(received_data);
        }

        // Menutup koneksi dengan klien
        close(new_socket);
        std::cout << "Connection closed with " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;
    }
}

int main() {
    std::string ipAddress = "127.0.0.1"; // Alamat IP untuk server
    int port = 6000;                     // Port untuk server

    // Memulai server dengan alamat IP dan port yang ditentukan
    startServer(ipAddress, port);

    return 0;
}
