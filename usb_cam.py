import cv2
tracker = cv2.TrackerCSRT.create()
rtsp_url = "rtsp://localhost:8554/tes"
videosumber= "/home/bagas/Downloads/video1.mp4"
usbcam= 2
video = cv2.VideoCapture(usbcam)
video.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
video.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
initialized = False
while True:
    ok, frame = video.read()
    # cv2.imshow("Tracking", frame)
    if not initialized:
        key = cv2.waitKey(1)  # Wait indefinitely until a key is pressed
        # if key == ord('s'):
        while not initialized:
            # bbox = cv2.selectROI("Tracking", frame, fromCenter=True, showCrosshair=True)
            bbox = (320,240,50,50)
            if bbox[2] == 0 or bbox[3] == 0:
                print("ROI selection cancelled")
                break
            tracker.init(frame, bbox) #initiator tracking
            initialized = True
        # elif key == 27:
        #     break 
    else:
        ok, bbox = tracker.update(frame)
        if ok:
            p1 = (int(bbox[0]), int(bbox[1]))
            p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
            cv2.rectangle(frame, p1, p2, (255, 0, 0), 2, 1)
            center_x = int(bbox[0] + bbox[2] / 2)
            center_y = int(bbox[1] + bbox[3] / 2)
            print(f"Center of bounding box: ({center_x}, {center_y})")
            cv2.circle(frame, (center_x, center_y), 5, (0, 255, 0), -1)
            cv2.line(frame, (center_x, 0), (center_x, frame.shape[0]), (0, 0, 255), 2)  # Vertical line
            cv2.line(frame, (0, center_y), (frame.shape[1], center_y), (0, 0, 255), 2)  # Horizontal line
        else:
            cv2.putText(frame, "Tracking failure detected", (100, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)
    # cv2.imshow("Tracking", frame)
    if cv2.waitKey(1) & 0xFF == 27:
        break
video.release()
cv2.destroyAllWindows()
