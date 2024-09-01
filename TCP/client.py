import socket
import numpy as np
import cv2

# Konfigurasi
host = '127.0.0.1'
port = 5000
addr = (host, port)

# Membuat socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(addr)

while True:
    # Menerima panjang data
    data_len = int.from_bytes(sock.recv(4), 'big')
    data = b''
    while len(data) < data_len:
        packet = sock.recv(data_len - len(data))
        if not packet:
            break
        data += packet

    # Mengonversi data gambar ke format gambar
    image = np.frombuffer(data, dtype=np.uint8)
    image = cv2.imdecode(image, cv2.IMREAD_COLOR)

    if image is not None:
        # Menampilkan gambar
        cv2.imshow('Received Video', image)
        if cv2.waitKey(1) == ord('q'):
            break

cv2.destroyAllWindows()
sock.close()
