import cv2
import time
tracker = cv2.TrackerCSRT.create()
rtsp_url = "rtsp://localhost:8554/tes"
videosumber = "/home/bagas/Downloads/video1.mp4"
usbcam = 2
video = cv2.VideoCapture(usbcam)
video.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
video.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
initialized = False
fps = 0
frame_count = 0
start_time = time.time()
while True:
    ok, frame = video.read()
    frame_count += 1
    current_time = time.time()
    elapsed_time = current_time - start_time
    if elapsed_time > 1.0:
        fps = frame_count / elapsed_time
        frame_count = 0
        start_time = current_time
    if not initialized:
        key = cv2.waitKey(1)  # Wait indefinitely until a key is pressed
        while not initialized:
            bbox = cv2.selectROI("Tracking", frame, fromCenter=True, showCrosshair=True)
            # bbox = (320,240,100,100)
            if bbox[2] == 0 or bbox[3] == 0:
                print("ROI selection cancelled")
                break
            tracker.init(frame, bbox) #initiator tracking
            initialized = True
    else:
        ok, bbox = tracker.update(frame)
        if ok:
            p1 = (int(bbox[0]), int(bbox[1]))
            p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
            cv2.rectangle(frame, p1, p2, (255, 0, 0), 2, 1)
            center_x = int(bbox[0] + bbox[2] / 2)
            center_y = int(bbox[1] + bbox[3] / 2)
            print(f"FPS {fps:.2f} Target: ({center_x}, {center_y})")
            cv2.circle(frame, (center_x, center_y), 5, (0, 0, 255), -1)
            cv2.line(frame, (center_x, 0), (center_x, frame.shape[0]), (0, 0, 255), 2)  # Vertical line
            cv2.line(frame, (0, center_y), (frame.shape[1], center_y), (0, 0, 255), 2)  # Horizontal line
        else:
            cv2.putText(frame, "Tracking failure detected", (100, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)
            print(f"FPS {fps:.2f} Missing Target")
    if cv2.waitKey(1) & 0xFF == 27:
        break
    cv2.imshow("Tracking", frame)
video.release()
cv2.destroyAllWindows()