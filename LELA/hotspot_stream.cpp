#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>

using namespace cv;
using namespace std;
using namespace std::chrono;

vector<vector<Point>> getContours(Mat hsv, Scalar lower, Scalar upper) {
    Mat mask;
    inRange(hsv, lower, upper, mask);
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    return contours;
}

Scalar parseHSV(const string& hsv_str) {
    int h, s, v;
    char sep;
    stringstream ss(hsv_str);
    ss >> h >> sep >> s >> sep >> v;
    return Scalar(h, s, v);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        cout << "Usage: " << argv[0] << " <lower_H,S,V> <upper_H,S,V> <IP> <port>" << endl;
        return -1;
    }

    Scalar lower_red = parseHSV(argv[1]); // HSV lower dari argumen pertama
    Scalar upper_red = parseHSV(argv[2]); // HSV upper dari argumen kedua
    string ip_address = argv[3];          // Alamat IP target dari argumen ketiga
    int port = stoi(argv[4]);             // Port target dari argumen keempat

    VideoCapture cap("/dev/video0");
    if (!cap.isOpened()) {
        cout << "Gagal membuka kamera." << endl;
        return -1;
    }

    bool recording = false;
    high_resolution_clock::time_point start_time;
    Size frame_size(640, 480);
    double fps = 30.0;

    // Mengganti alamat IP dan port di pipeline GStreamer sesuai dengan argumen
    string gst_pipeline = "appsrc ! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency ! rtph264pay config-interval=1 pt=96 ! udpsink host=" + ip_address + " port=" + to_string(port);
    
    VideoWriter video_writer(gst_pipeline, cv::CAP_GSTREAMER, 0, fps, frame_size, true);
    if (!video_writer.isOpened()) {
        cout << "Gagal membuka GStreamer pipeline." << endl;
        return -1;
    }

    while (true) {
        Mat frame;
        cap >> frame;
        resize(frame, frame, frame_size);

        if (frame.empty()) {
            cout << "Gagal membaca frame dari kamera." << endl;
            break;
        }

        Mat hsv;
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        vector<vector<Point>> contours = getContours(hsv, lower_red, upper_red);
        if (!contours.empty()) {
            vector<Point> largest_contour = *max_element(contours.begin(), contours.end(),
                [](const vector<Point>& a, const vector<Point>& b) {
                    return contourArea(a) < contourArea(b);
                });

            Rect bounding_rect = boundingRect(largest_contour);
            rectangle(frame, bounding_rect, Scalar(0, 255, 0), 2);
            putText(frame, "Hotspot", Point(bounding_rect.x, bounding_rect.y - 10),
                    FONT_HERSHEY_SIMPLEX, 0.9, Scalar(0, 255, 0), 2);

            if (!recording) {
                recording = true;
                start_time = high_resolution_clock::now();
            }
        }

        if (recording) {
            auto elapsed_time = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();
            if (elapsed_time >= 60) {
                recording = false;
            }
        }

        // Kirim frame melalui UDP menggunakan GStreamer
        video_writer.write(frame);

        if (waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    destroyAllWindows();

    return 0;
}
