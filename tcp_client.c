#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Cần: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    //kết nối server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR");
        return 1;
    }
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE); // Xóa buffer trước khi nhận
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        printf("Đã nhận từ server:\n%s\n", buffer);
    }
    printf("Gửi đến server:\n");
    while (1) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        if (strncmp(buffer, "exit", 4) == 0) break;
        
        send(sock, buffer, strlen(buffer), 0);
    }
    close(sock);
    return 0;
}