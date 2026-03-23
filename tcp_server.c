#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Cần: %s <PORT> <FILE chào của server> <FILE log>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    char *greeting_file = argv[2];
    char *output_file = argv[3];

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 1); //tối đa 1 client

    printf("Server connected at PORT: %d\n", port);

    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        
        // Đọc file chào và gửi
        FILE *g_file = fopen(greeting_file, "r");
        if (g_file) {
            char greeting[BUFFER_SIZE];
            size_t bytes_read = fread(greeting, 1, BUFFER_SIZE, g_file);
            send(client_sock, greeting, bytes_read, 0);
            fclose(g_file);
        }
        FILE *o_file = fopen(output_file, "a");
        if (o_file) {
            char buffer[BUFFER_SIZE];
            int bytes_received;
            
            while ((bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
                printf("Nhận từ client: %.*s", bytes_received, buffer);
                fwrite(buffer, 1, bytes_received, o_file);
                fflush(o_file); // Ghi ngay lập tức
            }
            fclose(o_file);
        }
        close(client_sock);
    }
    close(server_sock);
    return 0;
}