# NEXT TARGET 
1. Pick BBOX dengan live video OKE
2. STREAM video to RTSP OKE
3. Gabungkan program UDPstream dan Tracking OKE
3. receive video RTSP/UDP dan Pick ROI/BBOX dengan GCS OKEE
4. Kirim Data ROI/BBOX via ethernet OKEE
6. Gabungkan program tracking dan roireceiver

## Compile cpp
```shell
g++ -o tracking tracking.cpp `pkg-config --cflags --libs opencv4`
g++ -o rtsp_stream rtsp_stream.cpp `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0`
```

## Dependencies
```shell
sudo apt-get update && sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstrtspserver-1.0-dev
```