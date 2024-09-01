## Compile cpp
```shell
g++ -o rtsp rtsp.cpp `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0`
g++ hotspot_stream.cpp -o hotspot_stream `pkg-config --cflags --libs opencv4 gstreamer-1.0`
```
## Cara RUN 
```shell
./hotspot_stream <lower> <upper> <ip_adress> <port> <video.avi>
./rtsp <ip_adress> <device>
```
contohnya
```shell
./hotspot_stream 100,100,100 255,255,255 10.144.251.134 5000 video1.avi
./rtsp 10.144.54.221 /dev/video0
```