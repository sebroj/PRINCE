#include "load_data.h"

#include <cstdio>
#include <vector>
#include <string>

#include <array>

#define CHAR_BUF_SIZE 256

class DataPoints
{
private:
  std::vector<double> points1D;
  std::vector<std::array<double, 2>> points2D; // ew, std array

  void ProcessData1D(const std::vector<std::vector<double>>& data)
  {
  }
  void ProcessData2D(const std::vector<std::vector<double>>& data)
  {
    if (points2D.empty())
    {
      printf("empty, adding new data\n");
      for (const std::vector<double>& dataLine : data)
      {
        std::array<double, 2> point;
        point[0] = dataLine[0];
        point[1] = dataLine[1];

        points2D.push_back(point);
      }
    }
    else
    {
      printf("NOT empty... idk what to do\n");
    }
  }

public:
  void ProcessData(const std::vector<std::vector<double>>& data)
  {
    if (data.size() == 0)
      return; // TODO log error

    if (data[0].size() == 2)
      ProcessData1D(data);
    else if (data[0].size() == 3)
      ProcessData2D(data);
    else
      return; // TODO log error
  }
};

DataPoints dataPoints;

char* trim_whitespace(char* str)
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
std::vector<double> read_line_data(char* line)
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
      printf("    LINE ERROR - Malformed number: \"%s\"\n", start);
      return std::vector<double>();
    }

    data.push_back(value);
    start = ++delim;
  }

  return data;
}

bool load_data(const char* path, int dim, int typeID)
{
  //printf("Load %d-D data from %s\n", dim, path);
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  std::vector<std::vector<double>> data;
  char buf[CHAR_BUF_SIZE];
  int lineNumber = 1;
  while (fgets(buf, CHAR_BUF_SIZE, fp))
  {
    char* trimmed = trim_whitespace(buf);
    std::vector<double> lineData = read_line_data(trimmed);
    if (lineData.empty())
    {
      printf("Error reading line %d.\n", lineNumber);
      return false;
    }
    if (lineData.size() != dim + 1)
    {
      printf("    LINE ERROR - Read %d values, expected %d.\n",
        (int)lineData.size(), dim+1);
      printf("Error reading line %d.\n", lineNumber);
      return false;
    }

    data.push_back(lineData);
    lineNumber++;
  }

  dataPoints.ProcessData(data);

  return true;
}
