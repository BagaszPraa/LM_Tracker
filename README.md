# NEXT TARGET 
1. Pick BBOX dengan live video OKE
2. STREAM video to RTSP OKE
3. Gabungkan program RTSP server dan Tracking
3. receive video RTSP dan Pick ROI/BBOX dengan GCS
4. Kirim Data ROI/BBOX via ethernet

## Compile cpp
```shell
g++ -o tracking tracking.cpp `pkg-config --cflags --libs opencv4`
g++ -o rtsp_stream rtsp_stream.cpp `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0`
```

## Dependencies
```shell
sudo apt-get update && sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstrtspserver-1.0-dev
```