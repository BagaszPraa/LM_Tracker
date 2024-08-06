import cv2
import subprocess

# Fungsi untuk mengonversi frame ke RTSP stream menggunakan FFmpeg
def start_ffmpeg_streaming():
    # Perintah FFmpeg untuk mengirim aliran video ke RTSP server
    command = [
        'ffmpeg',
        '-y',  # Overwrite output files
        '-f', 'rawvideo',  # Input format
        '-vcodec', 'rawvideo',  # Input codec
        '-pix_fmt', 'bgr24',  # Input pixel format
        '-s', '640x480',  # Input size
        '-r', '30',  # Input framerate
        '-i', '-',  # Input from stdin
        '-c:v', 'libx264',  # Output codec
        '-pix_fmt', 'yuv420p',  # Output pixel format
        '-preset', 'ultrafast',  # Encoding speed
        '-f', 'rtsp',  # Output format
        'rtsp://localhost:8554/mystream'  # RTSP server URL
    ]
    
    # Jalankan perintah FFmpeg menggunakan subprocess
    return subprocess.Popen(command, stdin=subprocess.PIPE, stderr=subprocess.PIPE)

def main():
    # Buka kamera USB (0 untuk kamera default)
    cap = cv2.VideoCapture(2)
    
    if not cap.isOpened():
        print("Error: Unable to open camera")
        return

    # Tentukan resolusi dan framerate
    width = 640
    height = 480
    framerate = 30

    cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
    cap.set(cv2.CAP_PROP_FPS, framerate)

    # Mulai proses streaming dengan FFmpeg
    process = start_ffmpeg_streaming()

    while True:
        # Baca frame dari kamera
        ret, frame = cap.read()
        if not ret:
            print("Error: Unable to read from camera")
            break
        
        # Tulis frame ke stdin FFmpeg
        try:
            process.stdin.write(frame.tobytes())
        except BrokenPipeError:
            print("BrokenPipeError: FFmpeg process has terminated")
            break

        # Tampilkan frame untuk debugging
        cv2.imshow('USB Camera', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Lepaskan video capture dan tutup jendela
    cap.release()
    cv2.destroyAllWindows()
    process.terminate()

    # Cetak output dan error dari FFmpeg
    output, error = process.communicate()
    print("FFmpeg output:", output)
    print("FFmpeg error:", error)

if __name__ == "__main__":
    main()
