#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <iostream>
#include <chrono>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // Buka perangkat video (misalnya, kamera di /dev/video2)
    cv::VideoCapture cap("/dev/video2");
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // Membuat pipeline GStreamer
    GstElement *pipeline = gst_parse_launch(
        "appsrc name=mysrc ! videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay config-interval=1 name=pay0 pt=96 ! udpsink host=192.168.168.97 port=5000", NULL);
    GstElement *appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "mysrc");

    // Tentukan format video yang akan dikirimkan ke GStreamer
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "BGR",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT, 480,
                                        "framerate", GST_TYPE_FRACTION, 30, 1,
                                        NULL);
    gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);

    // Mulai streaming
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

        // Pemrosesan frame (misalnya, menambahkan anotasi)
        cv::rectangle(frame, cv::Point(100, 100), cv::Point(200, 200), cv::Scalar(0, 255, 0), 2);

        // Kirim frame ke pipeline GStreamer
        GstBuffer *buffer = gst_buffer_new_allocate(NULL, frame.total() * frame.elemSize(), NULL);
        gst_buffer_fill(buffer, 0, frame.data, frame.total() * frame.elemSize());

        GstFlowReturn ret;
        g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
        gst_buffer_unref(buffer);

        // Tampilkan frame untuk debugging (opsional)
        cv::imshow("Frame", frame);
        int key = cv::waitKey(1) & 0xFF;
        if (key == 27) {  // Tekan ESC untuk keluar
            break;
        }
    }

    // Tutup pipeline dan bersihkan
    gst_element_send_event(pipeline, gst_event_new_eos());
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
