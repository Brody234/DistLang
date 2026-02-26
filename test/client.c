#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(42069);

    // Turns local host or other IP from human readable to computer readable
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(client_fd);
        return 1;
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    const char *msg = "Hello from client!";

    ssize_t bytes_sent = send(client_fd, msg, strlen(msg), 0);
    if (bytes_sent == -1) {
        perror("send");
        close(client_fd);
        return 1;
    }

    printf("Sent %zd bytes to server.\n", bytes_sent);

    char buffer[1024];
    memset(&buffer, 0, sizeof(buffer));

    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received == -1) {
        perror("recv");
        close(client_fd);
        return 1;
    }

    if (bytes_received == 0) {
        printf("Server disconnected.\n");
    } else {
        buffer[bytes_received] = '\0';  // make it a C string
        printf("Received from server: %s\n", buffer);
    }

    // client central loop
    while(1){

    }

    close(client_fd);
    
    return 0;

}