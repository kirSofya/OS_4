#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int client_socket;

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

void exit_prigram() {
    printf("death\n");
    close(client_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    // проверка на количество входных данных
    if (argc != 4) {
        printf("Usage: <%s> <server ip> <sever port> <client id>\n", argv[0]);
        exit(1);
    }
    int id = atoi(argv[3]);
    struct sockaddr_in server_address;
    
    signal(SIGINT, exit_prigram);
    
    // создание клиентского сокета
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        DieWithError("socket() failed");
    }
    
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    
    // Преобразование IP-адреса
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        DieWithError("inet_pton() failed");
    }

    char buffer[1024] = "for_connect";
    // подключение к серверу
    sendto(client_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *)&server_address, sizeof(server_address));
    printf("Connection to the server: success\n");

    int stolen_items;

    while (1) {
        struct sockaddr_in server_res;
        socklen_t addrlen = sizeof(server_res);
        // получение количества украденных предметов
        if (recvfrom(client_socket, &stolen_items, sizeof(stolen_items), 0, (struct sockaddr *)&server_res, &addrlen) > 0) {
            if (stolen_items == -1) {
                break;
            }
            int cost = 0;
            if (id == 0) {
                printf("Ivanov stole\n");
            } else if (id == 1) {
                printf("Petrov loaded\n");
            } else if (id == 2) {
                printf("Necheporuk on the lookout\n");
                ++stolen_items;
                cost = rand() % 1000;
                printf("cost of item: %d\n", cost);
            }
            
            // отправка количества украденных предметов
            sendto(client_socket, &stolen_items, sizeof(stolen_items), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in));
            printf("stolen_items: %d, sent to server\n", stolen_items);
            
            // отправка стоимсоти предмета
            sendto(client_socket, &cost, sizeof(cost), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in));
        }
    }
    printf("finishing...\n");
    exit(0);
}
