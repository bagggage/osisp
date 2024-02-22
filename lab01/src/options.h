#pragma once

#include <cstdint>

enum Options : uint8_t {
    OP_DIRS = 0x01,
    OP_FILES = 0x02,
    OP_LINKS = 0x04,
    OP_SORT_BY_LOCALE = 0x08
};

static constexpr Options defaultOptions = static_cast<Options>(OP_DIRS | OP_FILES | OP_LINKS);