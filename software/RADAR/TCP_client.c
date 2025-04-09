#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>       // close()
#include <arpa/inet.h>    // socket, connect, sockaddr_in, inet_pton

#define SERVER_IP "192.168.0.6"
#define SERVER_PORT 5000
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("소켓 생성 실패");
        return 1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // IP 주소 문자열을 바이너리 형태로 변환
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("IP 주소 변환 실패");
        return 1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("서버 연결 실패");
        return 1;
    }

    printf("서버에 연결됨.\n");

    while (1) {
        printf("서버로 보낼 메시지 (종료하려면 'exit'): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // 줄바꿈 문자 제거
        buffer[strcspn(buffer, "\n")] = '\0';

        // 종료 조건
        if (strcmp(buffer, "exit") == 0)
            break;

        // 서버로 메시지 전송
        send(sock, buffer, strlen(buffer), 0);

        // 서버 응답 수신
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("서버 연결 종료 또는 오류\n");
            break;
        }

        buffer[bytes_received] = '\0';  // 문자열 끝 처리
        printf("서버로부터 받은 응답: %s\n", buffer);
    }

    close(sock);
    return 0;
}
