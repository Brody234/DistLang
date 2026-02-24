// Just playing with sockets and learning more about how they work.
// For anyone reading this log, I'm a data scientist cut me some slack.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

const int PORT_NUMBER = 42069;
const int BUFFER_SIZE = 1024;
const int BACKLOG = 20;
const int MAX_CONNECTIONS = 20;

int done[MAX_CONNECTIONS];
pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int id;
    int client_fd;
} connection_type;

void* client_handler(void* arg){
    connection_type info = *(connection_type*)arg; 
    free(arg);
    int client_fd = info.client_fd;

    char buffer[BUFFER_SIZE];
    
    // This just clears the buffer to 0
    memset(buffer, 0, sizeof(buffer));

    // Recieves Bytes
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(client_fd);
        return NULL;
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
        return NULL;
    }    
    printf("REPLIED");
    close(client_fd);
    printf("CLOSED");

    pthread_mutex_lock(&done_mutex);
        done[info.id] = 1;
    pthread_mutex_unlock(&done_mutex);
    return NULL;
}

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
    pthread_t thread[5];
    int i = 0;
    while(1 == 1){

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            close(server_fd);
            return 1;
        }

        connection_type *client_info = malloc(sizeof(connection_type));
        if (!client_info) {
            perror("malloc");
            close(client_fd);
            continue;
        }

        client_info->id = i;
        client_info->client_fd = client_fd;


        if (pthread_create(&thread[i], NULL, client_handler, client_info) != 0) {
            perror("pthread_create failed");
            return 1;
        }
        printf("here");

        for(int j = 0; j < 5; j++){
            if(done[j] == 1){
                pthread_join(thread[j], NULL);
                printf("Joined and exiting\n");
            }
        }
        printf("iter");
        i++;
        printf("%d", i);
    }
    
    close(server_fd);

    return 0;
}