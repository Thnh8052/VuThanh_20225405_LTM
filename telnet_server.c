#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9000
#define MAX_CLIENTS 100

int clients_state[MAX_CLIENTS]; // 0:chưa đăng nhập, 1: đã đăng nhập

// Hàm kiểm tra tài khoản từ file txt
int check_login(char *user, char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (!f) return 0;
    char f_user[32], f_pass[32];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
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
    memset(clients_state, 0, sizeof(clients_state));

    printf("Telnet Server is running on port %d...\n", PORT);

    while (1) {
        read_fds = all_fds;
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_sock) {
                    int client_sock = accept(server_sock, NULL, NULL);
                    FD_SET(client_sock, &all_fds);
                    if (client_sock > max_fd) max_fd = client_sock;
                    
                    clients_state[client_sock] = 0;
                    send(client_sock, "Nhap user va pass (cach nhau boi dau cach):\n", 44, 0);
                } else {
                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    int bytes_received = recv(i, buffer, sizeof(buffer) - 1, 0);

                    if (bytes_received <= 0) {
                        close(i);
                        FD_CLR(i, &all_fds);
                        clients_state[i] = 0;
                    } else {
                        buffer[strcspn(buffer, "\r\n")] = 0;

                        if (clients_state[i] == 0) {
                            //Xử lý login của client
                            char user[32], pass[32];
                            if (sscanf(buffer, "%s %s", user, pass) == 2 && check_login(user, pass)) {
                                clients_state[i] = 1;
                                send(i, "Dang nhap thanh cong. Nhap lenh:\n", 33, 0);
                            } else {
                                send(i, "Sai tai khoan hoac mat khau!\n", 29, 0);
                            }
                        } else {
                            // Xử lý lệnh client nhập
                            char cmd[1100];
                            sprintf(cmd, "%s > out.txt 2>&1", buffer); // Gộp cả lỗi vào out.txt
                            system(cmd);

                            // Đọc file out.txt và gửi lại client
                            FILE *f = fopen("out.txt", "r");
                            if (f) {
                                char file_buf[1024];
                                int len;
                                while ((len = fread(file_buf, 1, sizeof(file_buf), f)) > 0) {
                                    send(i, file_buf, len, 0);
                                }
                                fclose(f);
                            }
                            send(i, "\n>> ", 4, 0); // Prompt chờ lệnh tiếp
                        }
                    }
                }
            }
        }
    }
    return 0;
}