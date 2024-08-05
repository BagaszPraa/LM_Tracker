#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

int main() {
    // Create a CSRT tracker
    cv::Ptr<cv::TrackerCSRT> tracker = cv::TrackerCSRT::create();

    // Open the video capture device (index 2)
    cv::VideoCapture video(2);
    if (!video.isOpened()) {
        std::cerr << "Error: Could not open video device." << std::endl;
        return -1;
    }

    // Set the resolution to 640x480
    video.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    video.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // Variable to hold the ROI
    cv::Rect2d bbox;
    bool initialized = false;

    while (true) {
        cv::Mat frame;
        bool ok = video.read(frame);
        if (!ok) {
            std::cerr << "Error: Could not read frame from video." << std::endl;
            break;
        }

        // Show the frame
        cv::imshow("Tracking", frame);

        if (!initialized) {
            // Wait for user input
            char key = static_cast<char>(cv::waitKey(1));
            if (key == 's') {
                // Let the user select the ROI
                bbox = cv::selectROI("Tracking", frame, true, false);
                if (bbox.width == 0 || bbox.height == 0) {
                    std::cout << "ROI selection cancelled" << std::endl;
                    break;
                }
                // Initialize the tracker with the selected ROI
                tracker->init(frame, bbox);
                initialized = true;
            } else if (key == 27) { // ESC key
                break;
            }
        } else {
            // Update the tracker
            ok = tracker->update(frame, bbox);
            if (ok) {
                // Tracking success
                cv::rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);
                cv::Point2f center(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);
                std::cout << "Center of bounding box: (" << center.x << ", " << center.y << ")" << std::endl;
                cv::circle(frame, center, 5, cv::Scalar(0, 255, 0), -1);
                cv::line(frame, cv::Point(center.x, 0), cv::Point(center.x, frame.rows), cv::Scalar(0, 255, 0), 1);
                cv::line(frame, cv::Point(0, center.y), cv::Point(frame.cols, center.y), cv::Scalar(0, 255, 0), 1);
            } else {
                // Tracking failure
                cv::putText(frame, "Tracking failure detected", cv::Point(100, 80), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 255), 2);
            }
        }

        cv::imshow("Tracking", frame);

        // Exit if ESC is pressed
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    video.release();
    cv::destroyAllWindows();
    return 0;
}
