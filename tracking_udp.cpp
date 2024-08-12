#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <iostream>
#include <chrono>

cv::Rect roi;
bool selecting = false;
bool initialized = false;
cv::Rect roipick;
int fixed_width = 50;
int fixed_height = 50;
int width = 640;
int height = 320;

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <IP_ADDRESS>" << std::endl;
        return -1;
    }
    roi = cv::Rect(320,160,fixed_width,fixed_height);////////////////////////////////////////////////////////

    std::string ip_address = argv[1];

    gst_init(&argc, &argv);
    cv::Ptr<cv::Tracker> tracker = cv::TrackerCSRT::create();
    cv::VideoCapture cap("/dev/video2");
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cv::namedWindow("Frame");  
    cv::setMouseCallback("Frame", select_roi);

    // Membuat pipeline GStreamer dengan alamat IP dinamis
    std::string pipeline_str = "appsrc name=mysrc ! videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay config-interval=1 name=pay0 pt=96 ! udpsink host=" + ip_address + " port=5000";
    GstElement *pipeline = gst_parse_launch(pipeline_str.c_str(), NULL);
    GstElement *appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "mysrc");

    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "BGR",
                                        "width", G_TYPE_INT, width,
                                        "height", G_TYPE_INT, height,
                                        "framerate", GST_TYPE_FRACTION, 30, 1,
                                        NULL);
    gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    cv::Mat frame;
    int frame_count = 0;
    double fps = 0.0;
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        cap >> frame; // Ambil frame dari kamera
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
            }
        }

        GstBuffer *buffer = gst_buffer_new_allocate(NULL, frame.total() * frame.elemSize(), NULL);
        gst_buffer_fill(buffer, 0, frame.data, frame.total() * frame.elemSize());
        GstFlowReturn ret;
        g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
        gst_buffer_unref(buffer);

        // cv::imshow("Frame", frame);
        int key = cv::waitKey(1) & 0xFF;
        
        if (key == 27) {  // Tekan ESC untuk keluar
            break;
        }
        else if (key == 'r') {  // Tekan 'r' untuk reset ROI
            initialized = false;
            roi = cv::Rect2d();
            selecting = false;
            std::cout << "ROI reset." << std::endl;
        }
    }

    gst_element_send_event(pipeline, gst_event_new_eos());
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
