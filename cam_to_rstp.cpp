#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>  // For system()

int main() {
    // URL RTSP tujuan
    std::string rtsp_url = "rtsp://localhost:8554/live";

    // Membuka video capture dari USB kamera
    cv::VideoCapture cap(); // Gunakan 0 untuk kamera default
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video device." << std::endl;
        return -1;
    }

    // Menyiapkan FFmpeg untuk streaming
    std::string ffmpeg_cmd = "ffmpeg -y -f rawvideo -pix_fmt bgr24 -s 640x480 -r 30 -i - -c:v libx264 -f rtsp " + rtsp_url;
    FILE* ffmpeg_pipe = popen(ffmpeg_cmd.c_str(), "w");
    if (!ffmpeg_pipe) {
        std::cerr << "Error: Could not open FFmpeg pipe." << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
        // Membaca frame dari kamera
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame from video device." << std::endl;
            break;
        }

        // Menulis frame ke pipe FFmpeg
        fwrite(frame.data, 1, frame.total() * frame.elemSize(), ffmpeg_pipe);

        // Menampilkan frame untuk verifikasi
        cv::imshow("USB Camera Stream", frame);
        if (cv::waitKey(30) == 27) { // ESC untuk keluar
            break;
        }
    }

    // Menutup pipe FFmpeg dan video capture
    pclose(ffmpeg_pipe);
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
