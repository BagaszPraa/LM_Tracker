import cv2

# Buka stream menggunakan OpenCV
cap = cv2.VideoCapture("rtsp://192.168.168.16:5000")
# cap =cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open stream.")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Tampilkan frame
    cv2.imshow('RTSP Stream', frame)

    # Tekan 'q' untuk keluar
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
