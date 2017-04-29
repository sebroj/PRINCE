#include "parameters.h"

#include <cstdio>
#include <map>
#include "gsl/gsl_sf_bessel.h"

#include "node_main.h"
#include "param_io.h"
#include "exprtk_lite.h"

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

void ClearParam(const char* alias)
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

bool LoadParam(
  const char* alias, const char* path,
  int dim, std::array<CoordType, 2> coordTypes)
{
  DEBUGMsg("DBG: parameter: %s\n", alias);
  DEBUGMsg("     dimension: %d\n", dim);
  DEBUGMsg("     coords:    %d, %d\n", coordTypes[0], coordTypes[1]);
  DEBUGMsg("     filepath:  %s\n", path);
  
  std::vector<std::vector<double>> points;
  std::vector<double> values;
  if (!LoadData(alias, path, dim, points, values))
    return false;

  ParameterRaw rawParam(dim, coordTypes, points, values);
  InsertOrReplaceRawParam(alias, rawParam);

  return true;
}

bool LoadParam(const char* alias, const char* valueStr)
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