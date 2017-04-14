#pragma once

#include <vector>

// Data point coordinate indicators.
// The integer values matter, do NOT change them.
enum CoordType
{
  COORD_NONE = -1,
  COORD_X = 0,
  COORD_Y = 1,
  COORD_Z = 2
};

void clear_data(const char* alias);
bool load_data(
  const char* path, const char* alias,
  int dim, CoordType coordTypes[2]);
const std::vector<std::vector<double>>* get_points(const char* alias);
const std::vector<double>* get_values(const char* alias);

void to_regular_grid(
  const std::vector<std::vector<double>>& points,
  const std::vector<double>& values,
  std::vector<double>& out_values);
