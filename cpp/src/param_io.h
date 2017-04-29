#pragma once

#include <vector>

bool LoadData(
    const char* alias, const char* path, int dim,
    std::vector<std::vector<double>>& points,
    std::vector<double>& values);