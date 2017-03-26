#include "load_data.h"

#include <cstdio>
#include <vector>
#include <string>

#include <array>

#include "node_main.h"

// TODO all user errors have been labeled as "ERROR (USR): message"
// centralize this logging system.

class DataCoords
{
private:
  CoordType coordTypes[2] = { COORD_NONE, COORD_NONE };
  std::vector<double> coords1D;
  std::vector<std::array<double, 2>> coords2D; // TODO ew std::array

public:
  // Processes the new data points given by data.
  // Data must be sorted in ascending order (first by [0], then by [1], if 2D).
  bool new_data(const std::vector<std::vector<double>>& data)
  {
    if (data.empty())
    {
      DEBUG_error("new data was empty");
      return false;
    }
    int dataDim = (int)data[0].size() - 1;
    if (dataDim != 1 && dataDim != 2)
    {
      DEBUG_error("data points aren't 1-D or 2-D");
      return false;
    }

    int currentDim = dimension();
    if (dataDim == 1)
    {
      if (currentDim == 0)
      {
        printf("DBG: 1D, no previous points\n");
        for (const std::vector<double>& d : data)
          coords1D.push_back(d[0]);
      }
      else if (currentDim == 1)
      {
        printf("DBG: 1D, existing 1D points\n");
        for (int i = 0; i < coords1D.size(); i++)
        {
          if (coords1D[i] != data[i][0])
          {
            printf("ERROR (USR): New data points don't match previous data.\n.");
            return false;
          }
        }
      }
      else if (currentDim == 2)
      {
        printf("DBG: 1D, existing 2D points\n");
        for (int i = 0; i < coords2D.size(); i++)
        {
          // TODO determine which coordinate to check
          if (coords2D[i][0] != data[i][0])
          {
            printf("ERROR (USR): New data points don't match previous data.\n.");
            return false;
          }
        }
      }
    }
    else if (dataDim == 2)
    {
      if (currentDim == 0)
      {
        printf("DBG: 2D, no previous points\n");
        for (const std::vector<double>& d : data)
        {
          std::array<double, 2> coord = { d[0], d[1] };
          coords2D.push_back(coord);
        }
      }
      else if (currentDim == 1)
      {
        printf("DBG: 2D, existing 1D points (add stuff)\n");
        printf("UNIMPLEMENTED\n");
      }
      else if (currentDim == 2)
      {
        printf("DBG: 2D, existing 2D points\n");
        for (int i = 0; i < coords2D.size(); i++)
        {
          if (coords2D[i][0] != data[i][0] || coords2D[i][1] != data[i][1])
          {
            printf("ERROR (USR): New data points don't match previous data.\n.");
            return false;
          }
        }
      }
    }

    return true;
  }

  int dimension()
  {
    if (coords1D.empty() && coords2D.empty())
      return 0;
    else if (!coords1D.empty() && coords2D.empty())
      return 1;
    else if (coords1D.empty() && !coords2D.empty())
      return 2;

    DEBUG_error("both coords1D and coords2D are set");
    return -1;
  }
};

DataCoords dataCoords;

// Trim leading and trailing whitespace from str IN PLACE.
// Return a null-terminated substring of str with trimmed whitespace.
static char* trim_whitespace(char* str)
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
static std::vector<double> read_line_data(char* line)
{
  std::vector<double> data;

  char* start = line;
  char* delim = line;
  bool nullChar = false;
  while (!nullChar)
  {
    while (*delim != ',' && *delim != '\0')
      delim++;

    if (*delim == '\0')
      nullChar = true;
    *delim = '\0';

    start = trim_whitespace(start);
    char* endptr = start;
    double value = strtod(start, &endptr);
    if (endptr == start || *endptr != '\0')
    {
      printf("    LINE ERROR (USR): Malformed number \"%s\"\n", start);
      return std::vector<double>();
    }

    data.push_back(value);
    start = ++delim;
  }

  return data;
}

static bool compare_data(
  const std::vector<double>& d1,
  const std::vector<double>& d2)
{
  if (d1[0] == d2[0])
    return d1[1] <= d2[1];

  return d1[0] < d2[0];
}

bool load_data(const char* path, int dim, int paramID, CoordType coordTypes[2])
{
  const int BUF_SIZE = 256;

  printf("Load parameter %d's %d-D data from %s\n", paramID, dim, path);
  printf("Coordinates: %d, %d\n", coordTypes[0], coordTypes[1]);
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  std::vector<std::vector<double>> data;
  char buf[BUF_SIZE];
  int lineNumber = 1;
  while (fgets(buf, BUF_SIZE, fp))
  {
    char* trimmed = trim_whitespace(buf);
    std::vector<double> lineData = read_line_data(trimmed);
    if (lineData.empty())
    {
      printf("ERROR (USR): unable to read line %d\n", lineNumber);
      return false;
    }
    if (lineData.size() != dim + 1)
    {
      printf("    LINE ERROR (USR): Read %d values, expected %d.\n",
        (int)lineData.size(), dim+1);
      printf("ERROR (USR): unable to read line %d\n", lineNumber);
      return false;
    }

    data.push_back(lineData);
    lineNumber++;
  }

  // Sort data in the ascending order given by compare_data.
  std::sort(std::begin(data), std::end(data), compare_data);
  if (!dataCoords.new_data(data))
  {
    return false;
  }

  return true;
}
