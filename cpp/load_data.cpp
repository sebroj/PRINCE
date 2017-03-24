#include "load_data.h"

#include <cstdio>
#include <vector>
#include <string>

// TODO remove
#include <array>

#define CHAR_BUF_SIZE 256

bool load_file(const char* path, int dim)
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
    for (int i = 0; i < 3; i++)
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

    data.push_back(lineData);
  }

  return true;
}
