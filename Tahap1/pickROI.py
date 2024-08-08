import cv2

# Variabel global untuk menyimpan koordinat ROI
roi = None
selecting = False
start_point = (0, 0)
end_point = (0, 0)

# Callback fungsi mouse untuk memilih ROI
def select_roi(event, x, y, flags, param):
    global roi, selecting, start_point, end_point

    if event == cv2.EVENT_LBUTTONDOWN:
        selecting = True
        start_point = (x, y)
        end_point = (x, y)
    elif event == cv2.EVENT_MOUSEMOVE:
        if selecting:
            end_point = (x, y)
    elif event == cv2.EVENT_LBUTTONUP:
        selecting = False
        end_point = (x, y)
        roi = (start_point[0], start_point[1], end_point[0] - start_point[0], end_point[1] - start_point[1])

# Inisialisasi video capture dari webcam
cap = cv2.VideoCapture(2)

# Atur callback mouse
cv2.namedWindow("Frame")
cv2.setMouseCallback("Frame", select_roi)

# Inisialisasi tracker
tracker = cv2.TrackerCSRT.create()
initialized = False

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Gambar ROI saat memilih
    if selecting:
        cv2.rectangle(frame, start_point, end_point, (255, 0, 0), 2)

    # Inisialisasi tracker jika ROI telah dipilih
    if roi and not initialized:
        tracker.init(frame, roi)
        initialized = True

    # Update tracker
    if initialized:
        ok, bbox = tracker.update(frame)
        if ok:
            p1 = (int(bbox[0]), int(bbox[1]))
            p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
            cv2.rectangle(frame, p1, p2, (0, 255, 0), 2, 1)
            print(f"Bounding box: x={bbox[0]}, y={bbox[1]}, w={bbox[2]}, h={bbox[3]}")
        else:
            initialized = False
            roi = None
            print("Missing Target")
            cv2.putText(frame, "Tracking gagal", (100, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)

    # Tampilkan frame
    cv2.imshow("Frame", frame)

    # Break loop dengan menekan ESC
    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
