#include "parameters.h"

#include <cstdio>
#include <map>

#include "node_main.h"
#include "exprtk_lite.h"

#include "gsl/gsl_sf_bessel.h"

// TODO all user errors have been labeled as "ERROR (USR): message"
// centralize this logging system. Messages will be improved in future revision.

class ParameterRaw
{
public:
  int dim;
  std::array<CoordType, 2> coordTypes;
  std::vector<std::vector<double>> points;
  std::vector<double> values;

  ParameterRaw(
    int dim, std::array<CoordType, 2> coordTypes,
    const std::vector<std::vector<double>>& points,
    const std::vector<double>& values)
    : dim(dim), coordTypes(coordTypes), points(points), values(values)
  {
  }
};
std::map<std::string, ParameterRaw> rawParams;

// Stores information regarding the state of all plasma parameters.
class ParameterLinker
{
public:
  bool valid;
  int maxDim;
  std::array<CoordType, 2> coordTypes;
  std::map<std::string, std::vector<int>> indexMaps;
  std::vector<std::vector<double>> points;

  // Have some sort of mapping here for params of different dimensions?

  ParameterLinker()
    : valid(false)
  {
  }

  int MaxDimension()
  {
    int maxDimension = 0;
    for (const auto& it : rawParams) {
      int dim = it.second.dim;
      if (dim > maxDimension)
        maxDimension = dim;
    }

    return maxDimension;
  }

  void Update()
  {
    valid = false;
    maxDim = MaxDimension();
    coordTypes = { COORD_NONE, COORD_NONE };
    indexMaps.clear();
    points.clear();

    if (maxDim == 0) {
      for (const auto& it : rawParams)
        indexMaps.insert(decltype(indexMaps)::value_type(it.first, { 0 }));
    }
    else if (maxDim == 1) {
      // trickiest
      return;
    }
    else if (maxDim == 2) {
      for (const auto& it : rawParams) {
        // First pass: only scan through the 2-dimensional parameters
        const char* alias = it.first.c_str();
        const ParameterRaw& param = it.second;
        if (param.dim == 2) {
          if (points.empty()) {
            coordTypes = param.coordTypes;
            points = param.points;
            std::vector<int> indexMap(points.size());
            for (int i = 0; i < (int)indexMap.size(); i++) {
              indexMap[i] = i;
            }
            indexMaps.insert(decltype(indexMaps)::value_type(alias, indexMap));
          }
          else {
            if (coordTypes != param.coordTypes) {
              DEBUGMsg("ERROR (USR @ %s): coord type mismatch\n", alias);
              return;
            }
            else if (points != param.points) {
              DEBUGMsg("ERROR (USR @ %s): data points mismatch\n", alias);
              return;
            }
            else {
              std::vector<int> indexMap(points.size());
              for (int i = 0; i < (int)indexMap.size(); i++) {
                indexMap[i] = i;
              }
              indexMaps.insert(decltype(indexMaps)::value_type(alias, indexMap));
            }
          }
        }
      }
      for (const auto& it : rawParams) {
        // Second pass: scan through 1-D and 0-D parameters
        const char* alias = it.first.c_str();
        const ParameterRaw& param = it.second;
        if (param.dim == 1) {
          // TODO work in progress! this doesn't work yet
          if (param.coordTypes[0] == coordTypes[0]) {
            // check first coordinate matches all points
            // if it does:
            std::vector<int> indexMap(points.size());
            // TODO copied from node_main.cpp. centralize??
            /*int ptsHeight = 0;
            double first1 = points[0][0];
            while (points[++ptsHeight][0] == first1) {}
            int ptsWidth = (int)points.size() / ptsHeight;*/
            indexMaps.insert(decltype(indexMaps)::value_type(alias, indexMap));
          }
          else if (param.coordTypes[0] == coordTypes[1]) {
            // check second coordinate matches all points
            // if it does:
            std::vector<int> indexMap(points.size());
            indexMaps.insert(decltype(indexMaps)::value_type(alias, indexMap));
          }
          else {
            DEBUGMsg("ERROR (USR @ %s): coord type mismatch\n", alias);
            return;
          }
        }
        else if (param.dim == 0) {
          std::vector<int> indexMap(points.size(), 0);
          indexMaps.insert(decltype(indexMaps)::value_type(alias, indexMap));
        }
      }
    }

    valid = true;
  }
};
ParameterLinker paramLinker;

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

static void InsertOrReplaceRawParam(
  const char* alias,
  const ParameterRaw& rawParam)
{
  const auto& it = rawParams.find(alias);
  if (it != rawParams.end())
    it->second = rawParam;
  else
    rawParams.insert(decltype(rawParams)::value_type(alias, rawParam));

  paramLinker.Update();
}

void ClearData(const char* alias)
{
  DEBUGMsg("DBG: parameter: %s\n", alias);
  DEBUGMsg("     data cleared\n");

  if (rawParams.find(alias) != rawParams.end())
    rawParams.erase(alias);

  paramLinker.Update();

  // TODO debug code
	double x = 5.0;
	double y = gsl_sf_bessel_J0(x);
	DEBUGMsg("J0(%g) = %.18e\n\n", x, y);
}

bool LoadData(
  const char* alias, const char* path,
  int dim, std::array<CoordType, 2> coordTypes)
{
  const int BUF_SIZE = 256;

  DEBUGMsg("DBG: parameter: %s\n", alias);
  DEBUGMsg("     dimension: %d\n", dim);
  DEBUGMsg("     coords:    %d, %d\n", coordTypes[0], coordTypes[1]);
  DEBUGMsg("     filepath:  %s\n", path);
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  // Read raw data line by line.
  std::vector<std::vector<double>> data;
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
  std::vector<std::vector<double>> points;
  std::vector<double> values;
  for (const std::vector<double>& d : data) {
    std::vector<double> point;
    for (int i = 0; i < dim; i++)
      point.push_back(d[i]);

    points.push_back(point);
    values.push_back(d[dim]);
  }

  ParameterRaw rawParam(dim, coordTypes, points, values);
  InsertOrReplaceRawParam(alias, rawParam);

  return true;
}

bool LoadData(const char* alias, const char* valueStr)
{
  DEBUGMsg("DBG: parameter: %s\n", alias);
  DEBUGMsg("     dimension: 0\n");
  DEBUGMsg("     coords:    -1, -1\n");
  DEBUGMsg("     value string: %s\n", valueStr);

  char* endptr = (char*)valueStr;
  double value = strtod(valueStr, &endptr);
  if (endptr == valueStr || *endptr != '\0') {
    DEBUGMsg("ERROR (USR @ %s): Malformed number \"%s\"\n", alias, valueStr);
    return false;
  }

  std::vector<std::vector<double>> emptyPoints;
  std::vector<double> values = { value };
  ParameterRaw rawParam(0, { COORD_NONE, COORD_NONE }, emptyPoints, values);
  InsertOrReplaceRawParam(alias, rawParam);

  return true;
}

bool Calculate(
  const char* alias,
  const char* exprStr, std::vector<std::string> exprVars)
{
  DEBUGMsg("DBG: calculating %s\n", alias);
  DEBUGMsg("     expression: %s\n", exprStr);
  DEBUGMsg("     exprVars: %s\n", exprVars[0].c_str());
  for (int i = 1; i < (int)exprVars.size(); i++) {
    DEBUGMsg("               %s\n", exprVars[i].c_str());
  }

  for (std::string var : exprVars) {
    if (rawParams.find(var) == rawParams.end()) {
      DEBUGMsg("ERROR (USR @ %s): parameter not loaded\n", var.c_str());
      return false;
    }
  }
  if (!paramLinker.valid) {
    DEBUGMsg("ERROR (USR): parameters not linked\n");
    return false;
  }

  SymbolTable symbolTable;
  double* varValues = new double[exprVars.size()];
  for (int i = 0; i < (int)exprVars.size(); i++)
    symbolTable.AddVariable(exprVars[i], varValues[i]);
  
  Expression expr;
  expr.RegisterSymbolTable(symbolTable);

  Parser parser;
  if (!parser.Compile(exprStr, expr)) {
    DEBUGMsg("failed to compile expression %s", exprStr);
    return false;
  }

  std::vector<double> values(paramLinker.points.size());
  for (int p = 0; p < paramLinker.points.size(); p++) {
    // TODO this is TOTALLY inefficient. memory access all over the place
    for (int v = 0; v < (int)exprVars.size(); v++) {
      const ParameterRaw& param = rawParams.find(exprVars[v])->second;
      const std::vector<int>& indexMap =
        paramLinker.indexMaps.find(exprVars[v])->second;
      varValues[v] = param.values[indexMap[p]];
    }
    values[p] = expr.Value();
  }
  delete[] varValues;
  
  ParameterRaw rawParam(paramLinker.maxDim, paramLinker.coordTypes,
    paramLinker.points, values);
  InsertOrReplaceRawParam(alias, rawParam);

  return true;
}

const std::vector<std::vector<double>>* GetPoints(const char* alias)
{
  const auto& it = rawParams.find(alias);
  if (it == rawParams.end())
    return nullptr;

  return &(it->second.points);
}
const std::vector<double>* GetValues(const char* alias)
{
  const auto& it = rawParams.find(alias);
  if (it == rawParams.end())
    return nullptr;

  return &(it->second.values);
}

void ToRegularGrid(
  const std::vector<std::vector<double>>& points,
  const std::vector<double>& values,
  std::vector<double>& out_values)
{
  for (double value : values) {
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
      DEBUGError("new points were empty");
      return false;
    }
    int pointsDim = (int)points[0].size();
    if (pointsDim != 1 && pointsDim != 2)
    {
      DEBUGError("new points aren't 1-D or 2-D");
      return false;
    }

    int currentDim = dimension();
    if (pointsDim == 1)
    {
      if (currentDim == 0)
      {
        DEBUGMsg("DBG: 1D, no previous points\n");
        this->coordTypes[0] = coordTypes[0];
        // TODO make this a straight copy (must not pass values in points first)
        for (const std::vector<double>& point : points)
          coords1D.push_back(point[0]);
      }
      else if (currentDim == 1)
      {
        if (this->coordTypes[0] == coordTypes[0])
        {
          DEBUGMsg("DBG: 1D, existing 1D points, same coords\n");
          if (coords1D.size() != points.size())
          {
            DEBUGMsg("ERROR (USR): Number of new points doesn't match previous data.\n.");
            return false;
          }
          for (int i = 0; i < (int)coords1D.size(); i++)
          {
            if (coords1D[i] != points[i][0])
            {
              DEBUGMsg("ERROR (USR): New data points don't match previous data.\n.");
              return false;
            }
          }
        }
        else
        {
          DEBUGMsg("DBG: 1D, existing 1D points, different coords (convert)\n");
          DEBUGMsg("DBG: UNIMPLEMENTED\n");
        }
      }
      else if (currentDim == 2)
      {
        DEBUGMsg("DBG: 1D, existing 2D points\n");
        int coord;
        if (this->coordTypes[0] == coordTypes[0])
          coord = 0;
        else if (this->coordTypes[1] == coordTypes[0])
          coord = 1;
        else
        {
          DEBUGMsg("ERROR (USR): Input coordinate doesn't match existing data.\n");
          return false;
        }

        for (int i = 0; i < (int)coords2D.size(); i++)
        {
          if (coords2D.size() != points.size())
          {
            DEBUGMsg("ERROR (USR): Number of new points doesn't match previous data.\n.");
            return false;
          }
          if (coords2D[i][coord] != points[i][coord])
          {
            DEBUGMsg("ERROR (USR): New data points don't match previous data.\n.");
            return false;
          }
        }
      }
    }
    else if (pointsDim == 2)
    {
      if (currentDim == 0)
      {
        DEBUGMsg("DBG: 2D, no previous points\n");
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
        DEBUGMsg("DBG: 2D, existing 1D points (add stuff)\n");
        if (this->coordTypes[0] != coordTypes[0]
          && this->coordTypes[0] != coordTypes[1])
        {
          // TODO more info on these errors
          DEBUGMsg("ERROR (USR): Input coordinates don't match existing data.\n");
          return false;
        }
        DEBUGMsg("UNIMPLEMENTED\n");
      }
      else if (currentDim == 2)
      {
        DEBUGMsg("DBG: 2D, existing 2D points\n");
        if (this->coordTypes[0] != coordTypes[0]
          || this->coordTypes[1] != coordTypes[1])
        {
          // TODO more info on these errors
          DEBUGMsg("ERROR (USR): Input coordinates don't match existing data.\n");
          return false;
        }
        if (coords2D.size() != points.size())
        {
          DEBUGMsg("ERROR (USR): Number of new points doesn't match previous data.\n.");
          return false;
        }
        for (int i = 0; i < (int)coords2D.size(); i++)
        {
          if (coords2D[i][0] != points[i][0] || coords2D[i][1] != points[i][1])
          {
            DEBUGMsg("ERROR (USR): New data points don't match previous data.\n.");
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

    DEBUGError("both coords1D and coords2D are set");
    return -1;
  }
};
#endif
