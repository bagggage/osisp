#include <iostream>
#include <stdio.h>

#include "options.h"
#include "dirwalk.h"

int main(int argc, const char** argv) {
    Options options = defaultOptions;
    std::string directory;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (*arg == '-') {
            char c = *(++arg);

            do {
                switch (c)
                {
                case 'd':
                    options = static_cast<Options>(OP_DIRS | (options & OP_SORT_BY_LOCALE));
                    break;
                case 'l':
                    options = static_cast<Options>(OP_LINKS | (options & OP_SORT_BY_LOCALE));
                    break;
                case 'f':
                    options = static_cast<Options>(OP_FILES | (options & OP_SORT_BY_LOCALE));
                    break;
                case 's':
                    options = static_cast<Options>(options | OP_SORT_BY_LOCALE);
                    break;
                default:
                    std::cout << "Unknown optinon \'" << c << "\' in argument \"" << argv[i] << '\"' << std::endl;
                    break;
                }
            } while ((c = *(++arg)) != '\0');
        }
        else if (directory.empty()) {
            directory = arg;
        }
        else {
            std::cout << "Unknown argument: \'" << arg << '\"' << std::endl;
        }
    }

    if (directory.empty()) {
        directory = "./";
    }

    dirwalk(directory, options);

    return 0;
}