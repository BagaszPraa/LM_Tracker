# NEXT TARGET 
1. Pick BBOX dengan live video OKE
2. STREAM video to RTSP
3. Pick BBOX dengan GCS

## Compile cpp
```shell
g++ -o tracking tracking.cpp `pkg-config --cflags --libs opencv4`
g++ -o rtsp_stream rtsp_stream.cpp `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0`
```