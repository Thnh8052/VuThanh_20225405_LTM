#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE   1024
#define BACKLOG       5

//lấy thời gian hiện tại
static void get_timestamp(char *buf, int buf_size)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

//có client kết nối đến server
static void xu_ly_client(int client_sock,
                         const char *client_ip,
                         const char *log_file)
{
    char buf[BUFFER_SIZE];

    while (1)
    {
        int total = 0;

        // nhận dữ liệu
        while (total < (int)sizeof(buf) - 1) {
            int n = recv(client_sock, buf + total,
                         sizeof(buf) - 1 - total, 0);

            if (n <= 0) {
                printf("Client đã ngắt kết nối.\n");
                return; // thoát khi client disconnect
            }

            total += n;

            if (buf[total - 1] == '\n')
                break;
        }

        buf[total] = '\0';
        if (buf[total - 1] == '\n')
            buf[total - 1] = '\0';

        //phân tích chuỗi nhận được
        char mssv[20] = "", ho_ten[100] = "", ngay_sinh[15] = "", diem_str[20] = "";

        char tmp[BUFFER_SIZE];
        strncpy(tmp, buf, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';

        char *token = strtok(tmp, "|");
        if (token) strncpy(mssv, token, sizeof(mssv) - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(ho_ten, token, sizeof(ho_ten) - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(ngay_sinh, token, sizeof(ngay_sinh) - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(diem_str, token, sizeof(diem_str) - 1);

        if (!mssv[0] || !ho_ten[0] || !ngay_sinh[0] || !diem_str[0]) {
            printf("Sai định dạng: %s\n", buf);
            continue;
        }

        float diem_tb = atof(diem_str);

        char timestamp[30];
        get_timestamp(timestamp, sizeof(timestamp));

        char log_line[BUFFER_SIZE * 2];
        snprintf(log_line, sizeof(log_line),
                 "%s %s %s %s %s %.2f",
                 client_ip, timestamp, mssv, ho_ten, ngay_sinh, diem_tb);

        printf("\nClient %s đã gửi\n%s\n", client_ip, log_line);

        FILE *fp = fopen(log_file, "a");
        if (fp) {
            fprintf(fp, "%s\n", log_line);
            fclose(fp);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Cách dùng: %s <cổng> <file log>\n", argv[0]);
        fprintf(stderr, "Ví dụ    : %s 9090 sv_log.txt\n", argv[0]);
        return 1;
    }

    int   port     = atoi(argv[1]);
    const char *log_file = argv[2];

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Cổng không hợp lệ: %s\n", argv[1]);
        return 1;
    }
    //tạo socket TCP
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() thất bại");
        return 1;
    }

    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //gắn địa chỉ                                                      */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(port);

    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() thất bại");
        close(listener);
        return 1;
    }

    if (listen(listener, BACKLOG) == -1) {
        perror("listen() thất bại");
        close(listener);
        return 1;
    }

    printf("sv_server đang chạy trên cổng %d\n", port);
    printf("File log : %s\n", log_file);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        printf("Đang chờ kết nối mới...\n");

        int client_sock = accept(listener,
                                 (struct sockaddr *)&client_addr,
                                 &client_addr_len);
        if (client_sock == -1) {
            perror("accept() thất bại");
            continue;
        }

        //lấy địa chỉ IP của client
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr,
                  client_ip, sizeof(client_ip));

        printf("Client đã kết nối\n");

        //xử lý yêu cầu và ghi log
        xu_ly_client(client_sock, client_ip, log_file);

        close(client_sock);
        printf("Đã đóng kết nối với client\n\n");
    }
    close(listener);
    return 0;
}