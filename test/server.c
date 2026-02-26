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
const int MAX_CONNECTIONS_PENDING = 4;

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

    // Server central activity loop
    while(1){

    }
    
    close(client_fd);

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

    if (listen(server_fd, MAX_CONNECTIONS_PENDING) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Listening on port 0x%x\n", PORT_NUMBER);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread[MAX_CONNECTIONS];

    int started[MAX_CONNECTIONS];
    memset(&started, 0x0, sizeof(int)*MAX_CONNECTIONS);

    while(1){

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
        int i = -1;
        for(int k = 0; k < MAX_CONNECTIONS; k++){
            if(started[k] == 0){
                started[k] = 1;
                i = k;
                break;
            }
        }
        if(i == -1){
            const char *reply = "Maximum connections reached!";
            if (send(client_fd, reply, strlen(reply), 0) == -1) {
                perror("Failed to send disconnect");
                close(client_fd);
                continue;
            }    
            close(client_fd);
            continue;
        }

        printf("New thread at id %d\n", i);

        client_info->id = i;
        client_info->client_fd = client_fd;


        if (pthread_create(&thread[i], NULL, client_handler, client_info) != 0) {
            perror("pthread_create failed");
            return 1;
        }

        for(int j = 0; j < MAX_CONNECTIONS; j++){
            if(done[j] == 1){
                pthread_join(thread[j], NULL);
                done[j] = 0; // Should be impossible for any thread to use done
                started[j] = 0;
                printf("Joined and exiting\n");
            }
        }
    }
    
    close(server_fd);

    return 0;
}