#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>

class RTSPServer {
public:
    RTSPServer(const char* device) {
        server = gst_rtsp_server_new();
        gst_rtsp_server_set_service(server, "5000");

        GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
        GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

        gchar *launch_string = g_strdup_printf(
            "( v4l2src device=%s ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! videoconvert ! video/x-raw,format=I420 ! "
            "queue max-size-buffers=1 leaky=downstream ! "
            "x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay config-interval=1 name=pay0 pt=96 )", device);

        gst_rtsp_media_factory_set_launch(factory, launch_string);
        g_free(launch_string);
            
        gst_rtsp_media_factory_set_shared(factory, TRUE);
        gst_rtsp_mount_points_add_factory(mounts, "/live", factory);
        g_object_unref(mounts);
    }

    void run(const char* ip_address) {
        g_object_set(server, "address", ip_address, NULL);
        GMainLoop *loop = g_main_loop_new(NULL, FALSE);
        gst_rtsp_server_attach(server, NULL);
        g_print("RTSP server running at rtsp://%s:5000/live\n", ip_address);
        g_main_loop_run(loop);
    }

private:
    GstRTSPServer *server;
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        g_print("Usage: %s <IP_ADDRESS> <DEVICE>\n", argv[0]);
        return -1;
    }

    gst_init(&argc, &argv);

    RTSPServer server(argv[2]);
    server.run(argv[1]);

    return 0;
}
