#include "load_data.h"

#include <cstdio>
#include <vector>

#include <string>
#include <map>
#include <array>

#include "node_main.h"
//#include "extern/exprtk.h"

// TODO all user errors have been labeled as "ERROR (USR): message"
// centralize this logging system. Messages will be improved in future revision.

class Parameter
{
};

class ParameterRaw
{
  CoordType coordTypes[2];
  std::vector<std::vector<double>> points;
  std::vector<double> values;

public:
  ParameterRaw() {}

  ParameterRaw(
    CoordType coordTypes[2],
    const std::vector<std::vector<double>>& points,
    const std::vector<double>& values)
  {
    this->coordTypes[0] = coordTypes[0];
    this->coordTypes[1] = coordTypes[1];
    this->points = points;
    this->values = values;
  }

  bool is_set()
  {
    return !points.empty();
  }

  const std::vector<std::vector<double>>* get_points()
  {
    return &points;
  }
  const std::vector<double>* get_values()
  {
    return &values;
  }
};

std::map<std::string, ParameterRaw> rawParams;

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
static std::vector<double> read_line_doubles(char* line)
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
  if (d1[0] == d2[0] && d1.size() > 2)
    return d1[1] <= d2[1];
  return d1[0] < d2[0];
}

void clear_data(const char* alias)
{
  printf("DBG: parameter: %s\n", alias);
  printf("     data cleared\n");

  ParameterRaw paramRaw;
  rawParams[alias] = paramRaw;
}

bool load_data(
  const char* path, const char* alias,
  int dim, CoordType coordTypes[2])
{
  const int BUF_SIZE = 256;

  /*typedef exprtk::symbol_table<double> symbol_table_t;
  typedef exprtk::expression<double>   expression_t;
  typedef exprtk::parser<double>       parser_t;

  std::string expression_string = "clamp(-1.0,sin(2 * pi * x) + cos(x / 2 * pi),+1.0)";
  double x;

  symbol_table_t symbol_table;
  symbol_table.add_variable("x", x);
  symbol_table.add_constants();

  expression_t expression;
  expression.register_symbol_table(symbol_table);

  parser_t parser;
  parser.compile(expression_string,expression);

  for (x = double(-5); x <= double(+5); x += double(0.001))
  {
    double y = expression.value();
    printf("%19.15f\t%19.15f\n",x,y);
  }*/

  printf("DBG: parameter: %s\n", alias);
  printf("     dimension: %d-D\n", dim);
  printf("     coords:    %d, %d\n", coordTypes[0], coordTypes[1]);
  printf("     filepath:  %s\n", path);
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  // Read raw data line by line.
  std::vector<std::vector<double>> data;
  char buf[BUF_SIZE];
  int lineNumber = 1;
  while (fgets(buf, BUF_SIZE, fp))
  {
    char* trimmed = trim_whitespace(buf);
    std::vector<double> lineData = read_line_doubles(trimmed);
    if (lineData.empty())
    {
      printf("ERROR (USR): unable to read line %d\n", lineNumber);
      return false;
    }
    if ((int)lineData.size() != dim + 1)
    {
      printf("    LINE ERROR (USR): Read %d values, expected %d.\n",
        (int)lineData.size(), dim + 1);
      printf("ERROR (USR): unable to read line %d\n", lineNumber);
      return false;
    }

    data.push_back(lineData);
    lineNumber++;
  }

  // Sort data in the ascending order given by compare_data.
  std::sort(std::begin(data), std::end(data), compare_data);

  // TODO check if data points are complete (for 2D grid)
  // example implementation: for every data point, reverse (x, y) coords,
  // insert these to new list, sort this list, then check if newList == dataPts

  // Separate data into points and values.
  std::vector<std::vector<double>> points;
  std::vector<double> values;
  for (const std::vector<double>& d : data)
  {
    std::vector<double> point;
    for (int i = 0; i < dim; i++)
      point.push_back(d[i]);

    points.push_back(point);
    values.push_back(d[dim]);
  }

  ParameterRaw parameterRaw(coordTypes, points, values);
  rawParams[alias] = parameterRaw;

  return true;
}

const std::vector<std::vector<double>>* get_points(const char* alias)
{
  if (!rawParams[alias].is_set())
    return nullptr;

  return rawParams[alias].get_points();
}
const std::vector<double>* get_values(const char* alias)
{
  if (!rawParams[alias].is_set())
    return nullptr;

  return rawParams[alias].get_values();
}

void to_regular_grid(
  const std::vector<std::vector<double>>& points,
  const std::vector<double>& values,
  std::vector<double>& out_values)
{
  for (double value : values)
  {
    out_values.push_back(value);
  }
}

#if 0
class DataPoints
{
  CoordType coordTypes[2] = { COORD_NONE, COORD_NONE };
  std::vector<double> coords1D;
  std::vector<std::array<double, 2>> coords2D; // TODO ew std::array

public:
  // Processes the new data points given by points.
  // Points must be sorted in ascending order (first by [0], then by [1], if 2D)
  bool new_points(
    const std::vector<std::vector<double>>& points,
    CoordType coordTypes[2])
  {
    if (points.empty())
    {
      DEBUG_error("new points were empty");
      return false;
    }
    int pointsDim = (int)points[0].size();
    if (pointsDim != 1 && pointsDim != 2)
    {
      DEBUG_error("new points aren't 1-D or 2-D");
      return false;
    }

    int currentDim = dimension();
    if (pointsDim == 1)
    {
      if (currentDim == 0)
      {
        printf("DBG: 1D, no previous points\n");
        this->coordTypes[0] = coordTypes[0];
        // TODO make this a straight copy (must not pass values in points first)
        for (const std::vector<double>& point : points)
          coords1D.push_back(point[0]);
      }
      else if (currentDim == 1)
      {
        if (this->coordTypes[0] == coordTypes[0])
        {
          printf("DBG: 1D, existing 1D points, same coords\n");
          if (coords1D.size() != points.size())
          {
            printf("ERROR (USR): Number of new points doesn't match previous data.\n.");
            return false;
          }
          for (int i = 0; i < (int)coords1D.size(); i++)
          {
            if (coords1D[i] != points[i][0])
            {
              printf("ERROR (USR): New data points don't match previous data.\n.");
              return false;
            }
          }
        }
        else
        {
          printf("DBG: 1D, existing 1D points, different coords (convert)\n");
          printf("DBG: UNIMPLEMENTED\n");
        }
      }
      else if (currentDim == 2)
      {
        printf("DBG: 1D, existing 2D points\n");
        int coord;
        if (this->coordTypes[0] == coordTypes[0])
          coord = 0;
        else if (this->coordTypes[1] == coordTypes[0])
          coord = 1;
        else
        {
          printf("ERROR (USR): Input coordinate doesn't match existing data.\n");
          return false;
        }

        for (int i = 0; i < (int)coords2D.size(); i++)
        {
          if (coords2D.size() != points.size())
          {
            printf("ERROR (USR): Number of new points doesn't match previous data.\n.");
            return false;
          }
          if (coords2D[i][coord] != points[i][coord])
          {
            printf("ERROR (USR): New data points don't match previous data.\n.");
            return false;
          }
        }
      }
    }
    else if (pointsDim == 2)
    {
      if (currentDim == 0)
      {
        printf("DBG: 2D, no previous points\n");
        this->coordTypes[0] = coordTypes[0];
        this->coordTypes[1] = coordTypes[1];
        for (const std::vector<double>& point : points)
        {
          std::array<double, 2> coord = { { point[0], point[1] } };
          coords2D.push_back(coord);
        }
      }
      else if (currentDim == 1)
      {
        printf("DBG: 2D, existing 1D points (add stuff)\n");
        if (this->coordTypes[0] != coordTypes[0]
          && this->coordTypes[0] != coordTypes[1])
        {
          // TODO more info on these errors
          printf("ERROR (USR): Input coordinates don't match existing data.\n");
          return false;
        }
        printf("UNIMPLEMENTED\n");
      }
      else if (currentDim == 2)
      {
        printf("DBG: 2D, existing 2D points\n");
        if (this->coordTypes[0] != coordTypes[0]
          || this->coordTypes[1] != coordTypes[1])
        {
          // TODO more info on these errors
          printf("ERROR (USR): Input coordinates don't match existing data.\n");
          return false;
        }
        if (coords2D.size() != points.size())
        {
          printf("ERROR (USR): Number of new points doesn't match previous data.\n.");
          return false;
        }
        for (int i = 0; i < (int)coords2D.size(); i++)
        {
          if (coords2D[i][0] != points[i][0] || coords2D[i][1] != points[i][1])
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
#endif
