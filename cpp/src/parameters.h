#pragma once

#include <vector>
#include <string>
#include <array>

// Data point coordinate indicators.
// The integer values matter, do NOT change them.
enum CoordType
{
  COORD_NONE = -1,
  COORD_X = 0,
  COORD_Y = 1,
  COORD_Z = 2
};

void ClearData(const char* alias);

bool LoadData(
  const char* alias, const char* path,
  int dim, std::array<CoordType, 2> coordTypes);
bool LoadData(const char* alias, const char* valueStr);

bool Calculate(
  const char* alias,
  const char* expr, std::vector<std::string> exprVars);

const std::vector<std::vector<double>>* GetPoints(const char* alias);

const std::vector<double>* GetValues(const char* alias);

void ToRegularGrid(
  const std::vector<std::vector<double>>& points,
  const std::vector<double>& values,
  std::vector<double>& out_values);
