#include "dirwalk.h"

#include <dirent.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

bool is_dotdot_dir(const char* dirName) {
    return (strcmp(dirName, ".") == 0) || (strcmp(dirName, "..") == 0);
}

void dirwalk(const std::string& directory, const Options options, bool isRoot) {
    dirent** entries;

    int entriesCount = ((options & OP_SORT_BY_LOCALE) != 0) ?
        scandir(directory.c_str(), &entries, NULL, &alphasort) :
        scandir(directory.c_str(), &entries, NULL, NULL);

    if (entriesCount == -1) {
        std::cout << "Can't open director \"" << directory << "\": " << strerror(errno) << std::endl;
        return;
    }

    for (int i = 0; i < entriesCount; ++i) {
        struct dirent* entry = entries[i];
        struct stat entryStats;
        std::string entryPath = directory + entry->d_name;

        if (lstat(entryPath.c_str(), &entryStats) != 0) {
            if (errno != EACCES) {
                std::cout << "Can't get entry stats \'" << entryPath << "\': " << strerror(errno) << std::endl;
                continue;
            }
        }

        bool isDotDotDir = is_dotdot_dir(entry->d_name);

        if (S_ISDIR(entryStats.st_mode) && (options & OP_DIRS) ||
            S_ISREG(entryStats.st_mode) && (options & OP_FILES) ||
            S_ISLNK(entryStats.st_mode) && (options & OP_LINKS)) {
                if (isDotDotDir == false) {
                    std::cout << entryPath << std::endl;
                }
                else if (isRoot) {
                    std::cout << entry->d_name << std::endl;
                }
        }

        if (isDotDotDir == false && S_ISDIR(entryStats.st_mode)) dirwalk(entryPath + '/', options, false);

        free(entry);
    }

    free(entries);
}