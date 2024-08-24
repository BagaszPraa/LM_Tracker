#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

cv::Rect roi;
bool selecting = false;
bool initialized = false;
cv::Rect roipick;
int roiWidth = 50;
int roiHeight = 50;
int width = 640;
int height = 360;

#define BUFFER_SIZE 1024

void processReceivedData(const std::string& data) {
    std::istringstream ss(data);
    std::string token;
    std::vector<int> values;

    while (std::getline(ss, token, ',')) {
        try {
            values.push_back(std::stoi(token));
        } catch (std::exception& e) {
            std::cerr << "Invalid data format: " << e.what() << std::endl;
            return;
        }
    }

    if (values.size() == 4) {
        roi = cv::Rect(values[0], values[1], values[2], values[3]);
        initialized = false;
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

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on " << ipAddress << ":" << port << std::endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::cout << "Connection from " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;

        while (true) {
            ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                break;
            }
            std::string received_data(buffer, bytes_read);
            processReceivedData(received_data);
        }

        close(new_socket);
        std::cout << "Connection closed with " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;
    }
}

static gboolean on_need_data(GstAppSrc *appsrc, guint unused_size, gpointer user_data) {
    cv::VideoCapture* cap = static_cast<cv::VideoCapture*>(user_data);
    cv::Mat frame;

    if (!cap->read(frame)) {
        return FALSE; // End of stream
    }

    GstBuffer *buffer = gst_buffer_new_allocate(NULL, frame.total() * frame.elemSize(), NULL);
    gst_buffer_fill(buffer, 0, frame.data, frame.total() * frame.elemSize());
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);

    return TRUE;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <VIDEO_IP_ADDRESS> <VIDEO_PORT> <TCP_IP_ADDRESS> <TCP_PORT>" << std::endl;
        return -1;
    }

    std::string video_ip_address = argv[1];
    int video_port = std::stoi(argv[2]);
    std::string tcp_ip_address = argv[3];
    int tcp_port = std::stoi(argv[4]);

    gst_init(&argc, &argv);

    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    std::string pipeline_str = "appsrc name=mysrc ! videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay config-interval=1 name=pay0 pt=96";
    gst_rtsp_media_factory_set_launch(factory, pipeline_str.c_str());

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);
    std::cout << "RTSP server listening on rtsp://localhost:8554/test" << std::endl;

    cv::Ptr<cv::Tracker> tracker = cv::TrackerCSRT::create();
    cv::VideoCapture cap("/dev/video2");
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

    // Get the pipeline's appsrc element and connect the need-data signal
    g_signal_connect(gst_bin_get_by_name(GST_BIN(gst_rtsp_media_factory_create_element(factory)), "mysrc"), "need-data", G_CALLBACK(on_need_data), &cap);

    std::thread server_thread(startServer, tcp_ip_address, tcp_port);
    server_thread.detach();

    cv::Mat frame;
    int frame_count = 0;
    double fps = 0.0;
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        frame_count++;
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = current_time - start_time;
        if (elapsed_seconds.count() > 1.0) {
            fps = frame_count / elapsed_seconds.count();
            std::cout << "FPS = " << fps << std::endl;
            frame_count = 0;
            start_time = current_time;
        }

        if (!roi.empty() && !initialized) {
            tracker->init(frame, roi);
            initialized = true;
        }

        if (initialized) {
            cv::Rect bbox;
            bool ok = tracker->update(frame, bbox);
            if (ok) {
                cv::rectangle(frame, bbox, cv::Scalar(0, 255, 0), 1);
                int center_x = static_cast<int>(bbox.x + bbox.width / 2);
                int center_y = static_cast<int>(bbox.y + bbox.height / 2);
                std::cout << "FPS " << fps << " Target: (" << center_x << ", " << center_y << ")" << std::endl;
                cv::circle(frame, cv::Point(center_x, center_y), 5, cv::Scalar(0, 0, 255), -1);
                cv::line(frame, cv::Point(center_x, 0), cv::Point(center_x, frame.rows), cv::Scalar(0, 0, 255), 1);
                cv::line(frame, cv::Point(0, center_y), cv::Point(frame.cols, center_y), cv::Scalar(0, 0, 255), 1);
            } else {
                initialized = false;
                roi = cv::Rect();
                std::cout << "FPS " << fps << " Target: Missing" << std::endl;
            }
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
