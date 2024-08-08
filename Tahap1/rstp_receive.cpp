#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::string rtsp_url = "rtsp://localhost:8554/tes"; // Ganti dengan URL RTSP Anda

    cv::VideoCapture cap(rtsp_url);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open RTSP stream" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame; // Capture frame-by-frame
        if (frame.empty()) {
            std::cerr << "Error: Empty frame" << std::endl;
            break;
        }

        cv::imshow("RTSP Stream", frame);

        if (cv::waitKey(1) >= 0) { // Exit if any key is pressed
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
