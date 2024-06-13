#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#undef TRUE
#define TRUE 1

#undef FALSE
#define FALSE 0

#define FILL_RECORDS_COUNT 10

typedef uint8_t bool_t;

typedef struct record_s {
    char name[80];
    char address[80];
    uint8_t semester;
} record_t;

typedef enum status_e {
    STATUS_OK = 0,
    STATUS_EMPTY,
    STATUS_LOCKED,
    STATUS_INVALID,
    STATUS_FAIL
} status_t;

typedef enum command_e {
    CMD_EXIT = 0,
    CMD_LST,
    CMD_GET,
    CMD_MOD,
    CMD_PUT
} command_t;

enum mod_operation
{
	NAME 	 = '1',
	ADDRESS	 = '2',
	SEMESTER = '3',
	END      = '4'
};

static record_t temp_record;
static bool_t is_modified = FALSE;
static int file;

static void print_status(const status_t status) {
    const char* status_str;

    switch (status)
    {
    case STATUS_EMPTY:
        status_str = "No entries in file";
        break;
    case STATUS_INVALID:
        status_str = "Invalid input or record number";
        break;
    case STATUS_LOCKED:
        status_str = "File is busy, try later";
        break;
    case STATUS_FAIL:
        status_str = "Operation failed";
        break;
    }

    printf(": %s\n", status_str);
}

uint32_t handle_menu() {
    printf(
        "\nChoose option:\n"
        "1. LST 2. GET 3. MOD 4. PUT\n"
        "0. EXIT\n"
        "> "
    );

    int64_t result = 0;

    while (scanf("%li", &result) < 1 || result < CMD_EXIT || result > CMD_PUT) {
        fflush(stdin);
        printf("Invalid input, retry: > ");
    }

    putchar('\n');

    return result;
}

static void try_open(const char* filename) {
    file = open(filename, O_RDWR);

    if (file < 0) perror("Failed to open file");
}

static status_t fill_data(const char* filename) 
{
    FILE* file = fopen(filename, "wb");
    if (file == NULL) 
	{
        perror("Cannot create file\n");
        return STATUS_FAIL;
    }

    record_t records[FILL_RECORDS_COUNT] = {
        {"name0", "address0", 0},
        {"name1", "address1", 1},
        {"name2", "address2", 2},
        {"name3", "address3", 3},
        {"name4", "address4", 4},
        {"name5", "address5", 5},
        {"name6", "address6", 6},
        {"name7", "address7", 7},
        {"name8", "address8", 8},
        {"name9", "address9", 9}
    };

    for (int i = 0; i < FILL_RECORDS_COUNT; i++) {
        fwrite(&records[i], sizeof(record_t), 1, file);
    }

    fclose(file);

    return STATUS_OK;
}

status_t lst_impl() {
    status_t result = STATUS_OK;

    struct flock lock = {
        .l_pid = 0,
        .l_len = 0,
        .l_start = 0,
        .l_whence = SEEK_SET,
        .l_type = F_RDLCK
    };

    if (fcntl(file, F_SETFL, &lock) < 0) return STATUS_LOCKED;

    struct stat stats;
    fstat(file, &stats);

    const uint64_t rec_count = stats.st_size / sizeof(record_t);

    lseek(file, 0, SEEK_SET);

    if (rec_count == 0) result = STATUS_EMPTY;
    for (uint32_t i = 0; i < rec_count; ++i) {
        if (read(file, (void*)&temp_record, sizeof(record_t)) < sizeof(record_t)) continue;

        printf("[%u] Strudent: %s: address: %s: semester: %u\n",
            i + 1, temp_record.name, temp_record.address, temp_record.semester);
    }

    lock.l_type = F_UNLCK;
    fcntl(file, F_SETLK, &lock);

    return result;
}

status_t read_rec(const uint32_t rec_idx, record_t* const buffer) {
    status_t result = STATUS_OK;

    struct flock lock = {
        .l_pid = 0,
        .l_len = sizeof(record_t),
        .l_start = rec_idx * sizeof(record_t),
        .l_whence = SEEK_SET,
        .l_type = F_RDLCK
    };

    if (fcntl(file, F_SETFL, &lock) < 0) return STATUS_LOCKED;

    lseek(file, rec_idx * sizeof(record_t), SEEK_SET);
    
    if (read(file, (void*)buffer, sizeof(record_t)) < 0) {
        result = STATUS_FAIL;
    }

    lock.l_type = F_UNLCK;
    fcntl(file, F_SETLK, &lock);

    return result;
}

status_t get_impl() {
    puts("Enter record number: ");

    uint64_t rec_num = 0;

    if (scanf("%lu", &rec_num) < 1 || rec_num < 1) return STATUS_INVALID;

    return read_rec(rec_num - 1, &temp_record);
}

status_t mod_impl() {
    bool_t flag_continue = TRUE;

    record_t* record = &temp_record;

    getchar();

    do {
        printf(
            "1. Name\n"
            "2. Address\n"
            "3. Semester\n"
            "4. Continue...\n"
            "> "
        );

        char ch = getchar();
        char str_buffer[80];

        switch(ch) 
		{
            case NAME: 
			{
                printf("New name: ");
                scanf("%s", str_buffer);

                strncpy(record->name, str_buffer, sizeof(str_buffer));
                rewind(stdin);

                is_modified = TRUE;

                break;
            }
            case ADDRESS: 
			{
                printf("New address: ");
                scanf("%s", str_buffer);

                strncpy(record->address, str_buffer, sizeof(str_buffer));

                is_modified = TRUE;

                break;
            }
            case SEMESTER: {
                uint8_t new_semester;

                printf("New semester: ");
                scanf("%hhu", &new_semester);

                record->semester = new_semester;
                is_modified = TRUE;

                break;
            }
			case END: {
                flag_continue = FALSE;
                break;
            }
            default: {
                flag_continue = FALSE;
                break;
            }
        }

        getchar();

    } while (flag_continue);

    return STATUS_OK;
}

status_t save_rec(const record_t* record, const uint32_t idx) {
    status_t result = STATUS_OK;

    struct flock lock = {
        .l_pid = 0,
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = idx * sizeof(record_t),
        .l_len = sizeof(record_t)
    };

    struct stat stat;

    if (fstat(file, &stat) < 0) return STATUS_FAIL;
    if (stat.st_size <= lock.l_start) return STATUS_INVALID;

    if (fcntl(file, F_SETFL, &lock) < 0) return STATUS_LOCKED;

    lseek(file, idx * sizeof(record_t), SEEK_SET);

    if (write(file, record, sizeof(record_t)) < sizeof(record_t)) {
        result = STATUS_FAIL;
    }

    lock.l_type = F_UNLCK;
    fcntl(file, F_SETLK, &lock);

    return result;
}

status_t put_impl() {
    if (is_modified == FALSE) {
        printf("No record selected and modified: select and modify record before.\n");
        return STATUS_OK;
    }

    puts("Enter record number to replace: ");

    uint64_t rec_num = 0;

    return STATUS_OK;
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        printf("%s: No input\n", argv[0]);
        return -1;
    }

    try_open(argv[1]);

    if (file < 0) {
        fill_data(argv[1]);
        return -2;
    }

    bool_t is_polling = TRUE;

    do {
        uint32_t cmd_num = handle_menu();
        status_t status = STATUS_OK;

        switch (cmd_num)
        {
        case CMD_EXIT:
            is_polling = FALSE;
            break;
        case CMD_LST: status = lst_impl(); break;
        case CMD_GET: status = get_impl(); break;
        case CMD_MOD: status = mod_impl(); break;
        case CMD_PUT: status = put_impl(); break;
        default: break;
        }

        if (status != STATUS_OK) print_status(status);
    } while (is_polling);
    

    return 0;
}