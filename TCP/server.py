import socket
import cv2
import numpy as np

# Konfigurasi
host = '127.0.0.1'
port = 5000
addr = (host, port)

# Membuat socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(addr)
sock.listen(1)

print(f"Listening on {host}:{port}")

conn, addr = sock.accept()
print(f"Connected by {addr}")

# Membuka video (bisa dari file atau kamera)
cap = cv2.VideoCapture(0)  # Ganti 0 dengan path file jika menggunakan video file

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Mengencode frame menjadi JPEG
    _, img_encoded = cv2.imencode('.jpg', frame)
    data = img_encoded.tobytes()

    # Mengirim panjang data dan data gambar
    conn.sendall(len(data).to_bytes(4, 'big') + data)

cap.release()
conn.close()
sock.close()
