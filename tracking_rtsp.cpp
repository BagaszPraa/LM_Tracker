#include <iostream>
#include <thread>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>


int main() {

    /********************************************************
     * First create a RTSP server that would read RTP/H264
     * from localhost UDP port 5004 and stream as RTSP to
     * rtsp://127.0.0.1:8554/test
     ********************************************************/
    gst_init(NULL, NULL);
    GMainLoop *serverloop = g_main_loop_new(NULL, FALSE);
    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, "( udpsrc port=5004 ! application/x-rtp,encoding-name=H264 ! rtph264depay ! h264parse ! rtph264pay name=pay0 )");
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    gst_rtsp_server_attach(server, NULL);
    std::thread serverloopthread(g_main_loop_run, serverloop);
    std::cout << "stream ready at rtsp://127.0.0.1:8554/test" << std::endl;



    /********************************************************
     * Now RTSP server is running in its own thread, let's 
     * create an opencv application reading from the  
     * camera,encoding into H264 and sending as RTP/H264 
     * to localhost UDP/5004
     ********************************************************/
    //cv::VideoCapture camera("videotestsrc is-live=1 ! video/x-raw,format=BGR,width=640,height=480,framerate=30/1 ! queue ! appsink drop=1", cv::CAP_GSTREAMER);
    cv::VideoCapture camera(2);
    if (!camera.isOpened()) {
        std::cerr << "Failed to open camera. Exiting" << std::endl;
    	g_main_loop_quit(serverloop);
    	serverloopthread.join();
        return -1;
    }

    float fps = camera.get(cv::CAP_PROP_FPS);
    int w = camera.get(cv::CAP_PROP_FRAME_WIDTH);
    int h = camera.get(cv::CAP_PROP_FRAME_HEIGHT);
    if (w%4) {
    	std::cerr << "Width is not a multiple of 4. Unsupported" << std::endl;
    	g_main_loop_quit(serverloop);
    	serverloopthread.join();
    	return(-1);
    }
    if (h%2) {
    	std::cerr << "Height is not a multiple of 2. Unsupported" << std::endl;
    	g_main_loop_quit(serverloop);
    	serverloopthread.join();
    	return(-1);
    }

    cv::VideoWriter rtph264_writer("appsrc ! queue ! videoconvert ! video/x-raw,format=I420 ! x264enc key-int-max=30 insert-vui=1 tune=zerolatency ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5004", cv::CAP_GSTREAMER, 0, fps, cv::Size(w, h));
    if (!rtph264_writer.isOpened()) {
        std::cerr << "Failed to open writer. Exiting" << std::endl;
    	g_main_loop_quit(serverloop);
    	serverloopthread.join();
        return -1;
    }

    while (true) {
        cv::Mat frame;
        camera >> frame;
        if (frame.empty()) {
            break;
        }
        rtph264_writer.write(frame);
    }

    rtph264_writer.release();
    camera.release();

    return 0;
}