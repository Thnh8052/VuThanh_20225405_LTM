#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 100

//lưu trạng thái client
struct Client {
    int fd;
    char id[32];
    int is_registered; // 0:chưa đăng nhập, 1:đã đăng nhập
} clients[MAX_CLIENTS];

void send_to_all(int sender_fd, int max_fd, fd_set *all_fds, char *msg) {
    for (int i = 0; i <= max_fd; i++) {
        if (FD_ISSET(i, all_fds) && i != sender_fd && i != 3) {
            send(i, msg, strlen(msg), 0);
        }
    }
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = { .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = INADDR_ANY };
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 10);

    fd_set read_fds, all_fds;
    FD_ZERO(&all_fds);
    FD_SET(server_sock, &all_fds);
    int max_fd = server_sock;

    memset(clients, 0, sizeof(clients));

    printf("Chat Server is running on port %d...\n", PORT);

    while (1) {
        read_fds = all_fds;
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_sock) {
                    // 1.Có Client mới kết nối
                    int client_sock = accept(server_sock, NULL, NULL);
                    FD_SET(client_sock, &all_fds);
                    if (client_sock > max_fd) max_fd = client_sock;
                    
                    clients[client_sock].fd = client_sock;
                    clients[client_sock].is_registered = 0;
                    char *prompt = "Vui long nhap theo cu phap: client_id: client_name\n";
                    send(client_sock, prompt, strlen(prompt), 0);
                } else {
                    // 2.Có dữ liệu từ Client
                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    int bytes_received = recv(i, buffer, sizeof(buffer) - 1, 0);

                    if (bytes_received <= 0) { // Client ngắt kết nối
                        close(i);
                        FD_CLR(i, &all_fds);
                        clients[i].is_registered = 0;
                    } else {
                        buffer[strcspn(buffer, "\r\n")] = 0; // Xóa ký tự xuống dòng
                        
                        if (clients[i].is_registered == 0) {
                            // Xử lý đăng ký ID
                            char id[32], name[32];
                            if (sscanf(buffer, "%31[^:]: %31s", id, name) == 2) {
                                strcpy(clients[i].id, id);
                                clients[i].is_registered = 1;
                                send(i, "Dang ky thanh cong!\n", 20, 0);
                            } else {
                                send(i, "Sai cu phap. Nhap lai: client_id: client_name\n", 46, 0);
                            }
                        } else {
                            // Xử lý chat (Broadcast)
                            char out_msg[2048];
                            time_t now = time(NULL);
                            struct tm *t = localtime(&now);
                            sprintf(out_msg, "%04d/%02d/%02d %02d:%02d:%02d %s: %s\n",
                                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                    t->tm_hour, t->tm_min, t->tm_sec,
                                    clients[i].id, buffer);
                            send_to_all(i, max_fd, &all_fds, out_msg);
                        }
                    }
                }
            }
        }
    }
    return 0;
}