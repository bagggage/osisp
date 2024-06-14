#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "packet.h"

#define CLIENT_PACK_DATA_SIZE 256

static char work_dir[256];

typedef int(*cmd_handler_t)(const char* args, const int sock, const Packet* server_pack, Packet* const pack_buffer);

static cmd_handler_t handlers[CMD_MAX] = { NULL };

typedef struct InAddress {
    char host[64];
    uint16_t port;
} InAddress;

typedef struct Command {
    MsgType type;
    char* args;
} Command;

int echo_impl(const char* args, const int sock, const Packet* server_pack, Packet* const pack_buffer) {
    pack_buffer->type = CMD_ECHO;
    pack_buffer->size = strlen(args);

    memcpy(pack_buffer->data, args, pack_buffer->size);

    if (write(sock, (void*)pack_buffer, sizeof(Packet) + pack_buffer->size) < 0) return -1;
    if (read(sock, (void*)server_pack, sizeof(Packet)) < sizeof(Packet)) return -1; 
    if (read(sock, (void*)pack_buffer->data, server_pack->size) < 0) return -1;

    pack_buffer->data[server_pack->size] = '\0';

    printf("%s\n", pack_buffer->data);

    return 0;
}

int info_impl(const char* args, const int sock, const Packet* server_pack, Packet* const pack_buffer) {
    pack_buffer->type = CMD_INFO;
    pack_buffer->size = 0;

    if (write(sock, (void*)pack_buffer, sizeof(Packet)) < 0) return -1;
    if (read(sock, (void*)server_pack, sizeof(Packet) + sizeof(ServerInfo)) < 0) return -1;

    if (server_pack->type != ANS_INFO) return -1;

    ServerInfo* const info = (ServerInfo*)server_pack->data;
    printf(
        "Server: %s\nConnections: %u\nTime: %s\n",
        info->device_name,
        info->conn_count,
        ctime(&info->time)
    );

    return 0;
}

int cd_impl(const char* args, const int sock, const Packet* server_pack, Packet* const pack_buffer) {
    pack_buffer->type = CMD_CD;
    pack_buffer->size = strlen(args);

    memcpy(pack_buffer->data, args, pack_buffer->size);

    if (write(sock, (void*)pack_buffer, sizeof(Packet) + pack_buffer->size) < 0) return -1;
    if (read(sock, (void*)server_pack, sizeof(Packet)) < sizeof(Packet)) return -1; 

    char* buffer = (char*)malloc(server_pack->size);
    int result = -1;

    do {
        if (server_pack->type == ANS_STR) {
            if (read(sock, (void*)buffer, server_pack->size) < 0) break;

            memcpy(work_dir, buffer, server_pack->size);
            work_dir[server_pack->size] = '\0';
        }
        else if (server_pack->type == ANS_ERROR) {
            printf("Failed to change director: %s\n", strerror(server_pack->error));
        }

        result = 0;
    } while (0);

    free((void*)buffer);
    return result;
}

int list_impl(const char* args, const int sock, const Packet* const server_pack, Packet* const pack_buffer) {
    pack_buffer->type = CMD_LIST;
    pack_buffer->size = 0;

    if (write(sock, (void*)pack_buffer, sizeof(Packet)) < sizeof(Packet)) return -1;
    if (read(sock, (void*)server_pack, sizeof(Packet) + sizeof(DirListData)) < sizeof(Packet)) return -1;
    if (server_pack->type != ANS_LIST) return -1;

    const DirListData* const data = (const DirListData*)server_pack->data;

    printf("count: %u\n", data->count);

    if (data->count == 0) return 0;

    DirEntry* const entries = (DirEntry*)malloc(server_pack->size);

    if (entries == NULL) return -1;
    if (read(sock, (void*)entries, server_pack->size) < server_pack->size) {
        free(entries);
        return -1;
    }

    const DirEntry* entry = entries;

    for (uint32_t i = 0; i < data->count; ++i) {
        printf("%.*s\n", entry->length, entry->name);
        entry = (const DirEntry*)(entry->name + entry->length);
    }

    free(entries);

    return 0;
}

static inline void init_handlers() {
    handlers[CMD_ECHO] = &echo_impl;
    handlers[CMD_INFO] = &info_impl;
    handlers[CMD_CD] = &cd_impl;
    handlers[CMD_LIST] = &list_impl;
}

int parse_address(const char* addr_str, InAddress* const addr) {
    int i = 0;
    for (; addr_str[i] != ':'; ++i) {
        if (i == _SC_HOST_NAME_MAX) return -1;
    }

    memcpy(addr->host, addr_str, i);
    addr->host[i] = '\0';
    addr->port = atoi(addr_str + i + 1);

    return 0;
}

Command read_cmd() {
    printf("%s> ", work_dir);

    static char cmd_buffer[1024] = { '\0' };
    Command result = { .type = CMD_NONE, .args = cmd_buffer };

    fgets(cmd_buffer, sizeof(cmd_buffer), stdin);

    uint32_t i = 0;
    while (!isspace(cmd_buffer[i]) && cmd_buffer[i] != '\0') ++i;

    if (cmd_buffer[i] == '\0') {
        cmd_buffer[++i] = '\0';
    }
    else {
        cmd_buffer[i++] = '\0';

        if (cmd_buffer[i] != '\0') {
            while (cmd_buffer[i] != '\n' && cmd_buffer[i] != '\0') ++i;
            cmd_buffer[i] = '\0';
        }
    }

    if (strcmp("LIST", cmd_buffer) == 0) result.type = CMD_LIST;
    else if (strcmp("ECHO", cmd_buffer) == 0) result.type = CMD_ECHO;
    else if (strcmp("INFO", cmd_buffer) == 0) result.type = CMD_INFO;
    else if (strcmp("QUIT", cmd_buffer) == 0) result.type = CMD_QUIT;
    else if (strcmp("CD", cmd_buffer) == 0) result.type = CMD_CD;

    return result;
}

int get_initial_packet(const int socket, Packet* const packet_buffer) {
    const InitialData* init_data = (InitialData*)packet_buffer->data;

    if (read(socket, (void*)packet_buffer, MAX_PACKET_SIZE) < 0) {
        perror("Recieve packet error");
        return -1;
    }

    if (packet_buffer->type != ANS_INIT) return -1;

    printf("Server: %s\n", init_data->info.device_name);
    strncpy(work_dir, init_data->work_dir.name, sizeof(work_dir));

    return 0;
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        printf("No server address provided\n");
        return -1;
    }

    InAddress addr;

    if (parse_address(argv[1], &addr) != 0) {
        printf("Incorrect address input: '%s'\n", argv[1]);
        return -1;
    }

    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Failet to open connection");
        return -2;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(addr.port);
    if (inet_pton(AF_INET, addr.host, &server_addr.sin_addr) < 0) {
        perror("Failed to convert address");
        return -1;
    }

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        return -2;
    }

    Packet* client_packet = (Packet*)malloc(MAX_PACKET_SIZE);
    Packet* server_packet = (Packet*)malloc(MAX_PACKET_SIZE);

    if (get_initial_packet(client_sock, server_packet) < 0) {
        close(client_sock);
        free(server_packet);
        free(client_packet);
        return -3;
    }

    init_handlers();

    while(1) {
        const Command cmd = read_cmd();

        if (cmd.type == CMD_QUIT) break;
        if (cmd.type > CMD_QUIT && cmd.type < ANS_INIT) {
            char* args = cmd.args + strlen(cmd.args) + 1;
            int result = handlers[cmd.type](args, client_sock, server_packet, client_packet);

            if (result < 0) printf("Command failed: %s\n", cmd.args);
        }
        else {
            printf("Unknown command: %s\n", cmd.args);
        }
    }

    close(client_sock);
    free(client_packet);
    free(server_packet);

    return 0;
}