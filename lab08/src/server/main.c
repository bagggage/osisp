#define _BSD_SOURCE

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "packet.h"

#define MAX_PATH 256

typedef int (*handler_t)(int sock, int* const dir_fd, char* const buffer, Packet* const packet);

static uint32_t conn_count = 0;
static int work_dir_fd = -1;

static handler_t handlers[CMD_MAX] = { NULL };

int gethostname(char* name, size_t namelen);

int init_work_dir() {
    char pathname[MAX_PATH] = { '\0' };
    getcwd(pathname, MAX_PATH);

    if ((work_dir_fd = open(pathname, O_RDONLY | O_DIRECTORY)) < 0) {
        perror("Failed to open CWD file descriptor");
        return -1;
    }
}

void get_server_info(ServerInfo* const info) {
    gethostname(info->device_name, sizeof(info->device_name));

    info->conn_count = conn_count;
    info->time = time(NULL);
}

void get_fd_path(char* const buffer, const uint32_t size, const int fd) {
    char link_path[MAX_PATH];
    sprintf(link_path, "/proc/self/fd/%i", fd);
    readlink(link_path, buffer, size);
}

int change_work_dir(const char* path, int* const curr_dir_fd) {
    int dir_fd;
    if ((dir_fd = openat(*curr_dir_fd, path, O_RDONLY | O_DIRECTORY)) < 0) return -1;

    if (*curr_dir_fd != work_dir_fd) close(*curr_dir_fd);

    *curr_dir_fd = dir_fd;

    return 0;
}

int send_init_data(int sock, char* buffer, int dir_fd) {
    Packet* pack = (Packet*)buffer;
    pack->type = ANS_INIT;

    InitialData* data = (InitialData*)pack->data;
    data->info.conn_count = 1;
    data->info.time = time(NULL);

    get_server_info(&data->info);
    get_fd_path(data->work_dir.name, MAX_PATH, dir_fd);

    pack->size = sizeof(InitialData) + strlen(data->work_dir.name);

    if (write(sock, (void*)buffer, sizeof(Packet) + pack->size) < 0) {
        perror("Failed to send init packet");
        return -1;
    }

    return 0;
}

int echo_handler(int sock, int* const dir_fd, char* buffer, Packet* const packet) {
    if (packet->size == 0) return 0;

    int result = -1;
    char* msg = (char*)malloc(packet->size);

    do {
        if (read(sock, (void*)msg, packet->size) < 0) break;

        packet->type = ANS_STR;

        if (write(sock, (void*)packet, sizeof(packet)) < 0) break;
        if (write(sock, (void*)msg, packet->size) < 0) break;
        
        result = 0;
    } while(0);

    free(msg);
    return result;
}

int info_handler(int sock, int* const dir_fd, char* buffer, Packet* const packet) {
    if (packet->size != 0) return -1;

    get_server_info((ServerInfo* const)packet->data);

    packet->type = ANS_INFO;
    packet->size = sizeof(ServerInfo);

    if (write(sock, (void*)packet, sizeof(Packet) + sizeof(ServerInfo)) < 0) return -1;

    return 0;
}

int cd_handler(int sock, int* const dir_fd, char* buffer, Packet* const packet) {
    if (packet->size == 0) return 0;

    int result = -1;
    char* dir_path = (char*)calloc(MAX_PATH, 1);

    do {
        if (read(sock, (void*)dir_path, packet->size) < 0) break;

        dir_path[packet->size] = '\0';

        if (change_work_dir(dir_path, dir_fd) < 0) {
            packet->type = ANS_ERROR;
            packet->error = errno;

            free(dir_path);
            dir_path = NULL;
        }
        else {
            get_fd_path(dir_path, MAX_PATH, *dir_fd);
            printf("Clinet: %i: work dir: %s\n", sock, dir_path);
            packet->type = ANS_STR;
            packet->size = strlen(dir_path);
        }

        if (write(sock, (void*)packet, sizeof(packet)) < 0) break;
        if (dir_path != NULL) {
            if (write(sock, (void*)dir_path, packet->size) < 0) break;
        }
        
        result = 0;
    } while(0);

    free(dir_path);
    return result;
}

int list_handler(int sock, int* const dir_fd, char* buffer, Packet* const packet) {
    if (packet->size > 0) return -1;

    char path_buffer[MAX_PATH] = { '\0' };
    get_fd_path(path_buffer, MAX_PATH , *dir_fd);

    DirListData* data = (DirListData*)packet->data;
    data->count = 0;

    DIR* dir = opendir(path_buffer);

    if (dir == NULL) return -1;

    char* dir_array = malloc(MAX_PATH);
    struct dirent* ent;
    DirEntry* current = (DirEntry*)dir_array;
    
    while ((ent = readdir(dir)) != NULL) {
        if (
            strcmp(ent->d_name, ".") == 0 ||
            strcmp(ent->d_name, "..") == 0
        ) continue;

        const uint32_t length = strlen(ent->d_name);

        packet->size += sizeof(DirEntry) + length;
        if (packet->size > MAX_PATH) {
            dir_array = realloc((void*)dir_array, packet->size);
        }

        current->length = length;
        memcpy(current->name, ent->d_name, length);

        current = (DirEntry*)((char*)current + sizeof(DirEntry) + length);

        data->count++;
    }

    int result = -1;

    do {
        packet->type = ANS_LIST;

        if (write(sock, (void*)packet, sizeof(Packet) + sizeof(DirListData)) < 0) break;
        if (write(sock, (void*)dir_array, packet->size) < 0) break;

        result = 0;
    } while(0);

    free(dir_array);
    return result;
}

void* polling_routine(void* sock_in) {
    int sock = (int)sock_in;
    int local_work_dir = work_dir_fd;
    char buffer[MAX_PACKET_SIZE] = { '\0' };

    if (send_init_data(sock, buffer, local_work_dir) < 0) {
        close(sock);
        return NULL;
    }

    printf("Client %i: start polling\n", sock);

    //int error;
    //socklen_t len = sizeof(error);

    do {
        if (read(sock, buffer, sizeof(Packet)) < sizeof(Packet)) {

            if (errno != 0) {
                strerror_r(errno, buffer, MAX_PATH);
                printf("Client %i: failed to recive packet: %s\n", sock, buffer);
            }

            break;
        }

        Packet* const packet = (Packet*)buffer;
        printf("Client %i: recived packet: type: %u: size: %u\n", sock, packet->type, packet->size);

        if (packet->type >= CMD_MAX || packet->type < 0) {
            printf("Client %i: invalid command code: %u\n", sock, packet->type);
            break;
        }

        if (handlers[packet->type](sock, &local_work_dir, buffer, packet) < 0) {
            strerror_r(errno, buffer, MAX_PATH);
            printf("Client %i: command failed: %s", sock, buffer);
            break;
        }

        //if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) != 0) error = -1;
    } while(1);

    close(sock);
    conn_count--;

    printf("Client %i: disconnected\n", sock);

    pthread_exit(NULL);
}

void poll_commands(int sock) {
    pthread_t thread;

    if (pthread_create(&thread, NULL, &polling_routine, (void*)sock) < 0) {
        perror("Failed to start polling for client");
        close(sock);
    }
    if (pthread_detach(thread) < 0) {
        perror("Failed to detach thread");
    }

    conn_count++;
}

int main() {;
    if (init_work_dir() < 0) return -1;

    handlers[CMD_ECHO] = &echo_handler;
    handlers[CMD_INFO] = &info_handler;
    handlers[CMD_CD] = &cd_handler;
    handlers[CMD_LIST] = &list_handler;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sock < 0) {
        perror("Can't open socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset((void*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to create connection");
        return -2;
    }
    if (listen(server_sock, 256) < 0) {
        perror("Failed to start listening");
        return -2;
    }

    {
        struct sockaddr_in info_addr;
        socklen_t addr_len = sizeof(info_addr);

        if (getsockname(server_sock, (struct sockaddr*)&info_addr, &addr_len) < 0) {
            perror("Failed to get listening port");
            return -2;
        }

        printf("Listening on port: %u\n", ntohs(info_addr.sin_port));
    }

    while(1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) {
            perror("Failed to accept connection");
            return -3;
        }

        printf("Client %i: connected\n", client_sock);
        poll_commands(client_sock);
    }

    close(server_sock);
    close(work_dir_fd);

    return 0;
}