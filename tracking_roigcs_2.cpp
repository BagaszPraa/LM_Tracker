#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

using namespace cv;
using namespace std;
using namespace std::chrono;
Rect roi;
bool selecting = false;
bool initialized = false;
int width = 640;
int height = 360;

#define BUFFER_SIZE 1024

void processReceivedData(const string& data) {
    istringstream ss(data);
    string token;
    vector<int> values;
    // Memisahkan data yang diterima berdasarkan koma
    while (getline(ss, token, ',')) {
        try {
            values.push_back(stoi(token));  // Mengonversi string ke integer
        } catch (exception& e) {
            cerr << "Invalid data format: " << e.what() << endl;
            return;
        }
    }
    // Mengecek apakah ada 4 nilai yang diterima
    if (values.size() == 4) {
        roi = cv::Rect(values[0], values[1], values[2], values[3]);
        initialized = false;  // Reset tracker so it can be re-initialized
        // cout << "Received: x=" << values[0] << ", y=" << values[1]
        //           << ", width=" << values[2] << ", height=" << values[3] << endl;
    } else {
        cerr << "Invalid number of values received: " << values.size() << endl;
    }
}

void startServer(const string& ipAddress, int port) {
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
    cout << "Server listening on " << ipAddress << ":" << port << endl;
    while (true) {
        // Menerima koneksi dari klien
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        cout << "Connection from " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << endl;
        // Menerima data dari klien
        while (true) {
            ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                break;
            }
            // Mengubah buffer menjadi string dan memproses data
            string received_data(buffer, bytes_read);
            processReceivedData(received_data);
        }
        // Menutup koneksi dengan klien
        close(new_socket);
        cout << "Connection closed with " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Usage: " << argv[0] << " <IP> <port> <TCP_IP> <TCP_PORT>" << endl;
        return -1;
    }
    string ip_address = argv[1];          // Alamat IP target dari argumen ketiga
    int port = stoi(argv[2]);             // Port target dari argumen keempat
    string tcp_ip_address = argv[3];
    int tcp_port = stoi(argv[4]);
    VideoCapture cap("/home/bagas/Videos/FPV.mp4");
    // VideoCapture cap("/dev/video3");
    Ptr<Tracker> tracker = TrackerCSRT::create();
    if (!cap.isOpened()) {
        cout << "Gagal membuka kamera." << endl;
        return -1;
    }
    Size frame_size(width, height);
    string gst_pipeline = "appsrc ! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency ! rtph264pay config-interval=1 pt=96 ! udpsink host=" + ip_address + " port=" + to_string(port);
    VideoWriter video_writer(gst_pipeline, cv::CAP_GSTREAMER, 0, 30.0, frame_size, true);
    if (!video_writer.isOpened()) {
        cout << "Gagal membuka GStreamer pipeline." << endl;
        return -1;
    }
    int frame_count = 0;
    double fps = 0.0;
    auto start_time = steady_clock::now();
    thread server_thread(startServer, tcp_ip_address, tcp_port);
    server_thread.detach();

    while (true) {
        // Rect roi = Rect(100,100,100,100);
        Mat frame;
        cap >> frame;
        resize(frame, frame, frame_size);
        frame_count++;
        auto current_time = chrono::steady_clock::now();
        duration<double> elapsed_seconds = current_time - start_time;
        ///FPS print
        if (elapsed_seconds.count() > 1.0) {
            fps = frame_count / elapsed_seconds.count();
            cout << "FPS = " << fps << endl;
            frame_count = 0;
            start_time = current_time;
        }
        if (!roi.empty() && !initialized) {
            tracker->init(frame, roi);
            initialized = true;
        }
        if (initialized) {
            Rect bbox;
            bool ok = tracker->update(frame, bbox);
            if (ok) {
                rectangle(frame, bbox, Scalar(0, 255, 0),1);
                int center_x = static_cast<int>(bbox.x + bbox.width / 2);
                int center_y = static_cast<int>(bbox.y + bbox.height / 2);
                // cout << "FPS " << fps << " Target: (" << center_x << ", " << center_y << ")" << endl;
                line(frame, Point(center_x, 0), Point(center_x, frame.rows), Scalar(0, 0, 255), 1);  // Vertical line
                line(frame, Point(0, center_y), Point(frame.cols, center_y), Scalar(0, 0, 255), 1);  // Horizontal line
            } else {
                initialized = false;
                roi = Rect();
                cout << "FPS " << fps << " Target: Missing" << endl;
            }
        }
        if (frame.empty()) {
            cout << "Gagal membaca frame dari kamera." << endl;
            break;
        }
        video_writer.write(frame);
        if (waitKey(1) == 'q') {
            break;
        }
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
