#include <opencv2/opencv.hpp>
#include <iostream>
int main() {
    // Open the video capture device (usually index 0 for the default camera)
    cv::VideoCapture cap(2); // Change the index if you have multiple cameras
    // Check if the camera opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video device." << std::endl;
        return -1;
    }
    // Set the resolution to 640x480
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    // Create a window to display the video feed
    cv::namedWindow("Video Capture", cv::WINDOW_AUTOSIZE);
    cv::Mat frame;
    while (true) {
        // Capture a new frame
        cap >> frame;
        // Check if the frame is empty
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame from video device." << std::endl;
            break;
        }
        // Display the frame in the window
        cv::imshow("Video Capture", frame);
        // Exit if ESC key is pressed
        if (cv::waitKey(30) == 27) {
            break;
        }
    }
    // Release the video capture object and close the window
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
