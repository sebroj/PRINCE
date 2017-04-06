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

bool set_parameter_count(int count);
bool load_data(const char* path, int dim, int paramID, CoordType coordTypes[2]);
const std::vector<std::vector<double>>* get_points(int paramID);
const std::vector<double>* get_values(int paramID);
