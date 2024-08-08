import cv2
import time

def main():
    # Buka video dari kamera (0 untuk kamera default)
    cap = cv2.VideoCapture(2)
    
    if not cap.isOpened():
        print("Error: Unable to open camera")
        return
    
    # Variabel untuk menghitung FPS
    fps = 0
    frame_count = 0
    start_time = time.time()
    
    while True:
        # Baca frame dari video
        ret, frame = cap.read()
        if not ret:
            print("Error: Unable to read from camera")
            break
        
        # Hitung FPS
        frame_count += 1
        current_time = time.time()
        elapsed_time = current_time - start_time
        
        if elapsed_time > 1.0:
            fps = frame_count / elapsed_time
            frame_count = 0
            start_time = current_time
        
        # Tampilkan FPS pada frame
        # cv2.putText(frame, f'FPS: {fps:.2f}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2, cv2.LINE_AA)
        
        # Tampilkan frame dengan FPS
        cv2.imshow('Video Stream', frame)
        print(f"{fps:.2f}")
        
        # Tekan 'q' untuk keluar
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    # Lepaskan video capture dan tutup jendela
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
