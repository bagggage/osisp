#pragma once

#include <stdint.h>

#define MAX_PACKET_SIZE 512

typedef enum MsgType {
    CMD_NONE = -2,

    CMD_QUIT = -1,
    CMD_ECHO,
    CMD_INFO,
    CMD_CD,
    CMD_LIST,
    CMD_MAX,

    ANS_INIT,
    ANS_INFO,
    ANS_LIST,
    ANS_STR,
    ANS_ERROR,
} MsgType;

typedef struct Packet {
    union {
        uint32_t size;
        uint32_t error;
    };
    
    MsgType type;

    char data[];
} Packet;

typedef struct ServerInfo {
    uint32_t conn_count;
    time_t time;

    char device_name[128];
} ServerInfo;

typedef struct DirEntry {
    uint8_t length;
    char name[];
} DirEntry;

typedef struct DirListData {
    uint32_t count;
    DirEntry entries[];
} DirListData;

typedef struct InitialData {
    ServerInfo info;
    DirEntry work_dir;
} InitialData;