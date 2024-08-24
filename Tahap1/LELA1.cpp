#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>
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
int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Gagal membuka kamera." << endl;
        return -1;
    }
    bool recording = false;
    high_resolution_clock::time_point start_time;
    Size frame_size(640, 480);
    double fps = 30.0;
    string gst_pipeline = "appsrc ! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency ! rtph264pay config-interval=1 pt=96 ! udpsink host=127.0.0.1 port=5000";
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
        Scalar lower_red(5, 100, 165);  // HSV
        Scalar upper_red(21, 255, 255); // HSV
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
        // imshow("Pengenalan Hotspot", frame);
        if (waitKey(1) == 'q') {
            break;
        }
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
