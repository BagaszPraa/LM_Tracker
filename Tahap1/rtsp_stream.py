#!/usr/bin/env python3

import gi
import sys
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')
from gi.repository import Gst, GstRtspServer, GObject

Gst.init(None)

class RTSPServer(GstRtspServer.RTSPServer):
    def __init__(self, port, device):
        super().__init__()
        self.set_service(port)
        self.get_mount_points().add_factory("/live", self.create_pipeline(device))

    def create_pipeline(self, device):
        factory = GstRtspServer.RTSPMediaFactory()
        launch_string = f"""
            ( v4l2src device={device} ! 
              video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 !
              videoconvert ! 
              x264enc tune=zerolatency speed-preset=ultrafast ! 
              rtph264pay name=pay0 pt=96 )
        """
        factory.set_launch(launch_string)
        factory.set_shared(True)
        return factory

def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <IP_ADDRESS> <PORT> <DEVICE>")
        return -1

    ip_address = sys.argv[1]
    port = sys.argv[2]
    device = sys.argv[3]

    server = RTSPServer(port, device)
    server.set_address(ip_address)
    loop = GObject.MainLoop()
    server.attach(None)
    print(f"RTSP server running at rtsp://{ip_address}:{port}/live")
    loop.run()

if __name__ == '__main__':
    main()
