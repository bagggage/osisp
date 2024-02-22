#pragma once

#include <string>

#include "options.h"

void dirwalk(const std::string& directory, const Options options, bool isRoot = true);