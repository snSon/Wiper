import socket

HOST = '192.168.0.6'  # 모든 인터페이스에서 수신
PORT = 5000      # 사용할 포트 번호

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
    server_socket.bind((HOST, PORT))
    server_socket.listen()
    print(f"서버가 {PORT} 포트에서 대기 중...")

    conn, addr = server_socket.accept()
    with conn:
        print(f"{addr} 연결됨.")
        while True:
            data = conn.recv(1024)
            if not data:
                break
            print("클라이언트로부터 받은 메시지:", data.decode())
            conn.sendall(data)  # 받은 데이터를 그대로 다시 보냄 (에코 서버)
