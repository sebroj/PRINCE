#include "param_io.h"

#include <string>
#include <algorithm>

#include "node_main.h"

// Trim leading and trailing whitespace from str IN PLACE.
// Return a null-terminated substring of str with trimmed whitespace.
static char* TrimWhitespace(char* str)
{
  // Leading whitespace
  while (isspace(*str))
    str++;

  // All spaces
  if (*str == '\0')
    return str;

  // Trailing whitespace
  char* end = str + strlen(str) - 1;
  while (end > str && isspace(*end))
    end--;
  *(end + 1) = 0;

  return str;
}

// Read null-terminated string line as a list of comma-separated doubles.
// Return a vector of the read doubles.
static std::vector<double> ReadLineDoubles(char* line)
{
  std::vector<double> data;

  char* start = line;
  char* delim = line;
  bool nullChar = false;
  while (!nullChar) {
    while (*delim != ',' && *delim != '\0')
      delim++;

    if (*delim == '\0')
      nullChar = true;
    *delim = '\0';

    start = TrimWhitespace(start);
    char* endptr = start;
    double value = strtod(start, &endptr);
    if (endptr == start || *endptr != '\0') {
      DEBUGMsg("    LINE ERROR (USR): Malformed number \"%s\"\n", start);
      return std::vector<double>();
    }

    data.push_back(value);
    start = ++delim;
  }

  return data;
}

static bool CompareData(
  const std::vector<double>& d1,
  const std::vector<double>& d2)
{
  if (d1[0] == d2[0] && d1.size() > 2)
    return d1[1] <= d2[1];
  return d1[0] < d2[0];
}

bool LoadData(
  const char* alias, const char* path, int dim,
  std::vector<std::vector<double>>& points,
  std::vector<double>& values)
{
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  // Read raw data line by line.
  std::vector<std::vector<double>> data;
  const int BUF_SIZE = 256;
  char buf[BUF_SIZE];
  int lineNumber = 1;
  while (fgets(buf, BUF_SIZE, fp)) {
    char* trimmed = TrimWhitespace(buf);
    std::vector<double> lineData = ReadLineDoubles(trimmed);
    if (lineData.empty()) {
      DEBUGMsg("ERROR (USR @ %s): unable to read line %d\n", alias, lineNumber);
      return false;
    }
    if ((int)lineData.size() != dim + 1) {
      DEBUGMsg("    LINE ERROR (USR @ %s): Read %d values, expected %d.\n",
        alias, (int)lineData.size(), dim + 1);
      DEBUGMsg("ERROR (USR @ %s): unable to read line %d\n", alias, lineNumber);
      return false;
    }

    data.push_back(lineData);
    lineNumber++;
  }

  // Sort data in the ascending order given by CompareData.
  std::sort(std::begin(data), std::end(data), CompareData);

  // TODO check if data points are complete (for 2D grid)

  // Separate data into points and values.
  for (const std::vector<double>& d : data) {
    std::vector<double> point;
    for (int i = 0; i < dim; i++)
      point.push_back(d[i]);

    points.push_back(point);
    values.push_back(d[dim]);
  }

  return true;
}