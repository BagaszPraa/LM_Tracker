#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>

class RTSPServer {
public:
    RTSPServer() {
        server = gst_rtsp_server_new();
        gst_rtsp_server_set_service(server, "5000");

        GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
        GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
        
        gst_rtsp_media_factory_set_launch(factory,
            "( v4l2src ! video/x-raw,width=640,height=480 ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");
        
        gst_rtsp_media_factory_set_shared(factory, TRUE);
        gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

        g_object_unref(mounts);
    }

    void run() {
        GMainLoop *loop = g_main_loop_new(NULL, FALSE);
        gst_rtsp_server_attach(server, NULL);
        g_print("RTSP server running at rtsp://localhost:5000/test\n");
        g_main_loop_run(loop);
    }

private:
    GstRTSPServer *server;
};

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    RTSPServer server;
    server.run();

    return 0;
}
