import cv2
import time
roi = None
selecting = False
start_point = (0, 0)
end_point = (0, 0)
cap = cv2.VideoCapture(0)
cv2.namedWindow("Frame")
tracker = cv2.TrackerCSRT.create()
initialized = False
fps = 0
frame_count = 0
start_time = time.time()
def select_roi(event, x, y, flags, param):
    global roi, selecting, start_point, roipick
    w = 100
    h = 100
    if event == cv2.EVENT_LBUTTONDOWN:
        selecting = True
    elif event == cv2.EVENT_MOUSEMOVE and cv2.EVENT_LBUTTONDOWN: 
        roipick = (x - w // 2, y - h // 2, w, h)   
    elif event == cv2.EVENT_LBUTTONUP:
        selecting = False
        roi = (x - w // 2, y - h // 2, w, h)   
while True:
    cv2.setMouseCallback("Frame", select_roi)
    ret, frame = cap.read()
    frame_count += 1
    current_time = time.time()
    elapsed_time = current_time - start_time
    if elapsed_time > 1.0:
        fps = frame_count / elapsed_time
        frame_count = 0
        start_time = current_time
    if not ret:
        break
    if selecting:
        cv2.rectangle(frame, roipick, (255, 0, 0), 2)
    if roi and not initialized:
        tracker.init(frame, roi)
        initialized = True
    if initialized:
        ok, bbox = tracker.update(frame)
        if ok:
            p1 = (int(bbox[0]), int(bbox[1]))
            p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
            cv2.rectangle(frame, p1, p2, (0, 255, 0), 2, 1)
            center_x = int(bbox[0] + bbox[2] / 2)
            center_y = int(bbox[1] + bbox[3] / 2)
            print(f"FPS {fps:.2f} Target: ({center_x}, {center_y})")
            cv2.circle(frame, (center_x, center_y), 5, (0, 0, 255), -1)
            cv2.line(frame, (center_x, 0), (center_x, frame.shape[0]), (0, 0, 255), 2)  # Vertical line
            cv2.line(frame, (0, center_y), (frame.shape[1], center_y), (0, 0, 255), 2)  # Horizontal line
        else:
            initialized = False
            roi = None
            print(f"FPS {fps:.2f} Target: Missing")
            cv2.putText(frame, "Tracking gagal", (100, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)
    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF
    if key == 27:  # Tekan ESC untuk keluar
        break
    elif key == ord('r'):  # Tekan 'r' untuk reset ROI
        initialized = False
        roi = None
        selecting = False
cap.release()
cv2.destroyAllWindows()
