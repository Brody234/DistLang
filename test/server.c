// Just playing with sockets and learning more about how they work.
// For anyone reading this log, I'm a data scientist cut me some slack.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int PORT_NUMBER = 42069;
const int BUFFER_SIZE = 1024;
const int BACKLOG = 20;
const int MAX_CONNECTIONS = 20;

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd == -1){
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);          // port 8080 (convert to network byte order)
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // listen on all local interfaces

    printf("Socket created: fd = %d\n", server_fd);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    printf("Bind successful on port 0x%x\n", PORT_NUMBER);

    if (listen(server_fd, MAX_CONNECTIONS) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Listening on port 0x%x\n", PORT_NUMBER);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Client connected!\n");

    char buffer[BUFFER_SIZE];
    
    // This just clears the buffer to 0
    memset(buffer, 0, sizeof(buffer));

    // Recieves Bytes
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    if (bytes_received == 0) {
        printf("Client disconnected.\n");
    } else {
        buffer[bytes_received] = '\0';  // make it a C string
        printf("Received from client: %s\n", buffer);
    }

    const char *reply = "Message received!";
    if (send(client_fd, reply, strlen(reply), 0) == -1) {
        perror("send");
        close(client_fd);
        close(server_fd);
        return 1;
    }    

    close(server_fd);
    close(client_fd);
    
    return 0;
}