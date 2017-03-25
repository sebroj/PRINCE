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

// Read null-terminated string line as a list of comma-separated doubles.
// Return a vector of the read doubles.
std::vector<double> read_line_data(char* line)
{
  std::vector<double> data;
  char* start = line;
  char* delim = line;
  while (true)
  {
    while (*delim != ',' && *delim != '\n' && *delim != '\0')
      delim++;

    *delim = '\0';
    double value;
    try {
      value = std::stod(start);
    }
    catch (const std::invalid_argument& ia) {
      printf("Misformatted number at (INSERT USEFUL INFO).\n");
      return data; // TODO error return value??
    }
    data.push_back(value);
  }
}

bool load_data(const char* path, int dim, int typeID)
{
  printf("Load %d-D data from %s\n", dim, path);

  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  std::vector<std::vector<double>> data;
  char buf[CHAR_BUF_SIZE];
  while (fgets(buf, CHAR_BUF_SIZE, fp))
  {
    std::vector<double> lineData;

    char* start = buf;
    char* delim = buf;
    for (int i = 0; i < (dim + 1); i++)
    {
      while (*delim != ',' && *delim != '\n' && *delim != '\0')
        delim++;

      *delim = '\0';
      // TODO exception handling (probably write my own string->double parser)
      lineData.push_back(std::stod(start));
      start = ++delim;

      if (delim - buf >= CHAR_BUF_SIZE)
        break;
    }

    if (lineData.size() != dim + 1)
    {
      printf("Incorrect number of data points.");
      return false;
    }

    data.push_back(lineData);
  }

  dataPoints.ProcessData(data);

  return true;
}
