import cv2
import numpy as np
import time
cap = cv2.VideoCapture(0)

def get_contours(hsv, lower, upper):
    mask = cv2.inRange(hsv, lower, upper)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)    
    return contours
    
 
    
recording = False
start_time = None
video_writer = None
frame_size = None
fps = 30.0 

while True:
    ret, frame = cap.read()
    frame = cv2.resize(frame, (360, 240))
    foto=frame
    
    if not ret:
        print("Gagal membaca frame dari kamera.")
        break

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    lower_red = np.array([5, 100, 165])
    upper_red = np.array([21, 255, 255])
    contours = get_contours(hsv, lower_red, upper_red)

    if contours:
        largest_contour = max(contours, key=cv2.contourArea)
         
        x, y, w, h = cv2.boundingRect(largest_contour)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)  # Gambar kotak
        cv2.putText(frame, 'Hotspot', (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)  # Tampilkan teks
        if not recording:
            
            recording = True
            start_time = time.time()
            frame_size = (frame.shape[1], frame.shape[0])
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # Codec video (mp4v untuk MP4)
            video_writer = cv2.VideoWriter(f'recording_{time.time()}.mp4', fourcc, fps, frame_size)
            
    if recording and time.time() - start_time >= 60:
        recording = False
        video_writer.release()
        
    if recording:
        video_writer.write(frame)

    cv2.imshow('Pengenalan Hotspot', frame)
    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()