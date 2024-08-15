import socket

def start_server(host='127.0.0.1', port=6000):
    """
    Memulai server TCP yang mendengarkan koneksi dari klien.
    
    :param host: Alamat IP di mana server mendengarkan. '0.0.0.0' berarti semua antarmuka.
    :param port: Port yang digunakan server untuk mendengarkan koneksi.
    """
    # Membuat socket TCP
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Mengikat socket ke alamat dan port
    server_socket.bind((host, port))
    
    # Mendengarkan koneksi
    server_socket.listen(5)
    print(f"Server listening on {host}:{port}")
    
    while True:
        # Menerima koneksi dari klien
        client_socket, client_address = server_socket.accept()
        print(f"Connection from {client_address}")
        
        # Menerima data dari klien
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            # Menampilkan data yang diterima
            print(data.decode('utf-8'))
        
        # Menutup koneksi dengan klien
        client_socket.close()
        print(f"Connection closed with {client_address}")

if __name__ == "__main__":
    start_server()
