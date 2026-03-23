#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
typedef struct {
    char mssv[20];
    char ho_ten[100];
    char ngay_sinh[15];
    float diem_tb;
} SinhVien;
static void doc_dong(const char *prompt, char *buf, int buf_size)
{
    printf("%s", prompt);
    if (fgets(buf, buf_size, stdin) == NULL)
        buf[0] = '\0';
    int len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Cần: %s <địa chỉ IP> <cổng>\n", argv[0]);
        return 1;
    }

    const char *server_ip   = argv[1];
    int         server_port = atoi(argv[2]);

    //tạo socket TCP
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        perror("socket() thất bại");
        return 1;
    }

    //điền địa chỉ server và kết nối
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Địa chỉ IP không hợp lệ: %s\n", server_ip);
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect() thất bại");
        close(sock);
        return 1;
    }

    printf("Đã kết nối đến server\n");

while (1)
{
    SinhVien sv;
    memset(&sv, 0, sizeof(sv));

    printf("\nNhập thông tin (gõ 'exit' để thoát)\n");

    doc_dong("Nhập MSSV: ", sv.mssv, sizeof(sv.mssv));
    if (strcmp(sv.mssv, "exit") == 0)
        break;

    doc_dong("Nhập Họ và tên: ", sv.ho_ten, sizeof(sv.ho_ten));
    doc_dong("Nhập Ngày sinh: ", sv.ngay_sinh, sizeof(sv.ngay_sinh));

    char diem_str[20];
    doc_dong("Nhập Điểm TB: ", diem_str, sizeof(diem_str));
    sv.diem_tb = atof(diem_str);

    // đóng gói dữ liệu gửi
    char payload[BUFFER_SIZE];
    int len = snprintf(payload, sizeof(payload),
                       "%s|%s|%s|%.2f\n",
                       sv.mssv,
                       sv.ho_ten,
                       sv.ngay_sinh,
                       sv.diem_tb);
    // gửi dữ liệu
    int sent = send(sock, payload, len, 0);
    if (sent <= 0) {
        perror("Lỗi gửi");
        break;
    }

    printf("Đã gửi: %s", payload);
}
    close(sock);
    return 0;
}