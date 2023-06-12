#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int server_socket;

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    close(server_socket);
    exit(1);
}

void exit_prigram() {
    printf("death\n");
    close(server_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    // проверка на количество входных данных
    if (argc != 4) {
        printf("Usage: <%s> <port> <n> <id>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int n = atoi(argv[2]);
    int id = atoi(argv[3]);
    
    // проверка на правильность id
    if (id != 3) {
        DieWithError("Incorrect id");
    }
    
    signal(SIGINT, exit_prigram);

    ssize_t for_read;
    struct sockaddr_in new_socket[id];
    struct sockaddr_in broadcast_addr;
    int addrlen = sizeof(broadcast_addr);

    // создание серверного сокета
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        DieWithError("socket() failed");
    }

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = INADDR_ANY;
    broadcast_addr.sin_port = htons(port);

    // привязка серверного сокета к IP-адресу и порту
    if (bind(server_socket, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
        DieWithError("bind() failed");
    }

    printf("Server is waiting for clients...\n");

    for (int i = 0; i < 3; ++i) {
        char buffer[1024];
        // подключение клиентов
        if ((recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&broadcast_addr, (socklen_t *)&addrlen)) < 0) {
            DieWithError("recvfrom() failed");
        }
        printf("Client %d connected\n", i + 1);
        
        // Сохранение клиента
        memcpy(&new_socket[i], &broadcast_addr, sizeof(broadcast_addr));
    }
    printf("all clients connected\n");

    int stolen_items = 0;
    int client_id = 0;
    int sum = 0;
    while (stolen_items < n) {
        for (int i = 0; i < 3; ++i) {
            struct sockaddr_in client_res;
            socklen_t client_addr = sizeof(client_res);
            sendto(server_socket, &stolen_items, sizeof(int), 0, (struct sockaddr *)&new_socket[client_id], sizeof(struct sockaddr_in));

            int res;
            // получаем значение количества украденных предметов
            recvfrom(server_socket, &res, sizeof(res), 0, (struct sockaddr *)&client_res, &client_addr);

            if (res != -1) {
                stolen_items = res;
            }
            // получаем стоимость украденного предмета
            recvfrom(server_socket, &res, sizeof(res), 0, (struct sockaddr *)&client_res, &client_addr);
            // добавляем в сумарную стоимость
            sum += res;
            if (res != 0) {
                printf("cost of %d item is %d\n", stolen_items, res);
            }
            // переходим к следующему клиенту
            client_id = (client_id + 1) % 3;
        }
    }
    
    printf("client_id = %d\n", client_id);
    printf("stolen_items = %d\n", stolen_items - 1);

    printf("TOTAL COST: %d\n", sum);
    int for_exit = -1;
    for (int i = 0; i < 3; ++i) {
        // завершение программы
        sendto(server_socket, &for_exit, sizeof(int), 0, (struct sockaddr *)&new_socket[i], sizeof(struct sockaddr_in));
    }
    close(server_socket);
    exit(0);
}
