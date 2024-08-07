#!/usr/bin/env python3

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')
from gi.repository import Gst, GstRtspServer, GObject

Gst.init(None)

class RTSPServer(GstRtspServer.RTSPServer):
    def __init__(self):
        super().__init__()
        self.set_service("5000")
        self.get_mount_points().add_factory("/test", self.create_pipeline())

    def create_pipeline(self):
        factory = GstRtspServer.RTSPMediaFactory()
        factory.set_launch("( v4l2src ! video/x-raw,width=640,height=480 ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )")
        factory.set_shared(True)
        return factory

def main():
    server = RTSPServer()
    loop = GObject.MainLoop()
    server.attach(None)
    print("RTSP server running at rtsp://localhost:5000/test")
    loop.run()

if __name__ == '__main__':
    main()
