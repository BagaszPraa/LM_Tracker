#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <iostream>
#include <chrono>

// Variabel global untuk menyimpan koordinat ROI
cv::Rect roi;
bool selecting = false;
bool initialized = false;
cv::Rect roipick;
int fixed_width = 100;
int fixed_height = 100;
cv::Ptr<cv::Tracker> tracker = cv::TrackerCSRT::create();
int frame_count = 0;
double fps = 0.0;
auto start_time = std::chrono::steady_clock::now();

// Callback fungsi mouse untuk memilih ROI
void select_roi(int event, int x, int y, int, void*) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        selecting = true;
    }
    else if (event == cv::EVENT_MOUSEMOVE && selecting) {
        roipick = cv::Rect(x - fixed_width / 2, y - fixed_height / 2, fixed_width, fixed_height);
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        selecting = false;
        roi = cv::Rect(x - fixed_width / 2, y - fixed_height / 2, fixed_width, fixed_height);
    }
}
int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cv::namedWindow("Frame");
    cv::setMouseCallback("Frame", select_roi);
    cv::Mat frame;
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
            frame_count = 0;
            start_time = current_time;
        }
        if (selecting) {
            cv::rectangle(frame, roipick, cv::Scalar(255, 0, 0), 2);
        }
        if (!roi.empty() && !initialized) {
            tracker->init(frame, roi);
            initialized = true;
        }
        if (initialized) {
            cv::Rect bbox;
            bool ok = tracker->update(frame, bbox);
            if (ok) {
                cv::rectangle(frame, bbox, cv::Scalar(0, 255, 0), 2, 1);
                int center_x = static_cast<int>(bbox.x + bbox.width / 2);
                int center_y = static_cast<int>(bbox.y + bbox.height / 2);
                std::cout << "FPS " << fps << " Target: (" << center_x << ", " << center_y << ")" << std::endl;
                cv::circle(frame, cv::Point(center_x, center_y), 5, cv::Scalar(0, 0, 255), -1);
                cv::line(frame, cv::Point(center_x, 0), cv::Point(center_x, frame.rows), cv::Scalar(0, 0, 255), 2);  // Vertical line
                cv::line(frame, cv::Point(0, center_y), cv::Point(frame.cols, center_y), cv::Scalar(0, 0, 255), 2);  // Horizontal line
            } else {
                initialized = false;
                roi = cv::Rect();
                std::cout << "FPS " << fps << " Target: Missing" << std::endl;
                cv::putText(frame, "Tracking gagal", cv::Point(100, 80), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 255), 2);
            }
        }
        cv::imshow("Frame", frame);
        int key = cv::waitKey(1) & 0xFF;
        if (key == 27) {  // Tekan ESC untuk keluar
            break;
        } else if (key == 'r') {  // Tekan 'r' untuk reset ROI
            initialized = false;
            roi = cv::Rect();
            selecting = false;
        }
    }
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
