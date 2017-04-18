// Jose M Rico
// March 15, 2017
// node_main.cpp

// Main entry point for the Node.js addon interface.
// Only this file should know anything about Node and V8.

#include "node_main.h"

#include <nan.h>

#include <vector>

#include "load_data.h"

using namespace Nan;

using v8::Local;

enum ValueTypes
{
  VALUE_INT,
  VALUE_STR,
  VALUE_ARRAY,
  VALUE_FUNC,
  VALUE_OBJ
};

static bool VerifyCbInfo(
  const char* funcName,
  const FunctionCallbackInfo<v8::Value>& cbInfo,
  const std::vector<ValueTypes>& valueTypes);
static bool VerifyArray(
  const Local<v8::Array>& array,
  const std::vector<ValueTypes>& valueTypes);

class StringConv
{
  const Utf8String str;

public:
  StringConv(const Local<v8::Value>& value)
    : str(value)
  {}

  const char* cstr() const
  {
    return *str ? *str : "STRING CONVERSION FAILED";
  }
};

NAN_METHOD(ClearParameter)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes({ VALUE_STR });
  if (!VerifyCbInfo(__FUNCTION__, cbInfo, cbInfoTypes))
    return;

  StringConv alias(cbInfo[0]);
  clear_data(alias.cstr());
}

NAN_METHOD(LoadParameter)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes(
    { VALUE_STR, VALUE_STR, VALUE_INT, VALUE_ARRAY });
  if (!VerifyCbInfo(__FUNCTION__, cbInfo, cbInfoTypes))
    return;

  // Verify array argument
  Local<v8::Array> array = cbInfo[3].As<v8::Array>();
  std::vector<ValueTypes> arrayTypes({ VALUE_INT, VALUE_INT });
  if (!VerifyArray(array, arrayTypes)) {
    DEBUGError("%s coordTypes array is misformatted.", __FUNCTION__);
    return;
  }
  int a0 = Nan::To<int>(Nan::Get(array, 0).ToLocalChecked()).FromJust();
  int a1 = Nan::To<int>(Nan::Get(array, 1).ToLocalChecked()).FromJust();
  if (a0 < COORD_NONE || a0 > COORD_Z || a1 < COORD_NONE || a1 > COORD_Z) {
    DEBUGError("%s coordTypes array elements should be CoordType values",
      __FUNCTION__);
    return;
  }

  // Translate arguments for load procedure.
  StringConv alias(cbInfo[0]);
  StringConv arg2(cbInfo[1]);
  int dataDim = Nan::To<int>(cbInfo[2]).FromJust();
  CoordType coordTypes[2] = { (CoordType)a0, (CoordType)a1 };

  bool success = false;
  if (dataDim == 0)
    success = load_data(alias.cstr(), arg2.cstr());
  else
    success = load_data(alias.cstr(), arg2.cstr(), dataDim, coordTypes);

  cbInfo.GetReturnValue().Set(success);
}

NAN_METHOD(CalcParameter)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes({ VALUE_STR, VALUE_STR });
  if (!VerifyCbInfo(__FUNCTION__, cbInfo, cbInfoTypes))
    return;

  StringConv alias(cbInfo[0]);
  StringConv expr(cbInfo[1]);
  bool success = calculate(alias.cstr(), expr.cstr());
  cbInfo.GetReturnValue().Set(success);
}

NAN_METHOD(GetParamData)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes({ VALUE_STR });
  if (!VerifyCbInfo(__FUNCTION__, cbInfo, cbInfoTypes))
    return;

  StringConv alias(cbInfo[0]);

  // Attempt to retrieve the data.
  const std::vector<double>* valuesPtr = get_values(alias.cstr());
  if (valuesPtr == nullptr) {
    cbInfo.GetReturnValue().Set(Null());
    return;
  }
  const std::vector<std::vector<double>>& points = *get_points(alias.cstr());
  const std::vector<double>& values = *valuesPtr;
  int dataDim = (int)points[0].size();

  Local<v8::Object> result = New<v8::Object>();

  Set(result, New<v8::String>("dim").ToLocalChecked(), New<v8::Int32>(dataDim));

  if (dataDim == 1) {
    // Transfer 1-D data to the V8 runtime.
    Local<v8::Array> data = New<v8::Array>();
    for (int i = 0; i < (int)points.size(); i++)
      Set(data, i, New<v8::Number>(points[i][0]));

    for (int i = 0; i < (int)values.size(); i++)
      Set(data, (int)points.size() + i, New<v8::Number>(values[i]));

    Set(result, New<v8::String>("data").ToLocalChecked(), data);
  }
  if (dataDim == 2) {
    int height = 0;
    double firstX = points[0][0];
    while (points[++height][0] == firstX) {}
    int width = (int)values.size() / height;
    Set(result, New<v8::String>("width").ToLocalChecked(),
      New<v8::Int32>(width));
    Set(result, New<v8::String>("height").ToLocalChecked(),
      New<v8::Int32>(height));

    std::vector<double> valuesRegular;
    to_regular_grid(points, values, valuesRegular);
    Local<v8::Array> data = New<v8::Array>();
    for (int i = 0; i < (int)valuesRegular.size(); i++) {
      // Change index from column-grouping to row-grouping.
      int row = i % height;
      int col = i / height;
      Set(data, row * width + col, New<v8::Number>(values[i]));
    }

    Set(result, New<v8::String>("data").ToLocalChecked(), data);
  }

  cbInfo.GetReturnValue().Set(result);
}

/* Receive all formats.json data, and any additional information needed. */
NAN_METHOD(Setup)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes(
    { VALUE_OBJ, VALUE_OBJ, VALUE_OBJ, VALUE_OBJ });
  if (!VerifyCbInfo(__FUNCTION__, cbInfo, cbInfoTypes))
    return;

  Local<v8::Object> plasmaParams = Nan::To<v8::Object>(cbInfo[0]).ToLocalChecked();
  Local<v8::Object> dispRels = Nan::To<v8::Object>(cbInfo[1]).ToLocalChecked();
  Local<v8::Object> constants = Nan::To<v8::Object>(cbInfo[2]).ToLocalChecked();
  Local<v8::Object> calcParams = Nan::To<v8::Object>(cbInfo[3]).ToLocalChecked();
}

static bool VerifyCbInfo(
  const char* funcName,
  const FunctionCallbackInfo<v8::Value>& cbInfo,
  const std::vector<ValueTypes>& valueTypes)
{
  if (cbInfo.Length() != (int)valueTypes.size()) {
    DEBUGError("%s expected %d arguments", funcName, (int)valueTypes.size());
    return false;
  }
  // TODO this possibly still uses non-NAN stuff.
  for (int i = 0; i < cbInfo.Length(); i++) {
    if (valueTypes[i] == VALUE_INT) {
      if (!cbInfo[i]->IsInt32()) {
        DEBUGError("%s arg %d should be an int", funcName, i+1);
        return false;
      }
    }
    else if (valueTypes[i] == VALUE_STR) {
      if (!cbInfo[i]->IsString()) {
        DEBUGError("%s arg %d should be a string", funcName, i+1);
        return false;
      }
    }
    else if (valueTypes[i] == VALUE_ARRAY) {
      if (!cbInfo[i]->IsArray()) {
        DEBUGError("%s arg %d should be an array", funcName, i+1);
        return false;
      }
    }
    else if (valueTypes[i] == VALUE_FUNC) {
      if (!cbInfo[i]->IsFunction()) {
        DEBUGError("%s arg %d should be a function", funcName, i+1);
        return false;
      }
    }
    else if (valueTypes[i] == VALUE_OBJ) {
      if (!cbInfo[i]->IsObject()) {
        DEBUGError("%s arg %d should be an object", funcName, i+1);
        return false;
      }
    }
  }

  return true;
}

static bool VerifyArray(
  const Local<v8::Array>& array,
  const std::vector<ValueTypes>& valueTypes)
{
  if (array->Length() != (int)valueTypes.size())
    return false;

  // TODO this possibly still uses non-NAN stuff.
  for (int i = 0; i < (int)array->Length(); i++) {
    Local<v8::Value> ai = Nan::Get(array, i).ToLocalChecked();
    if (valueTypes[i] == VALUE_INT)
      if (!ai->IsInt32())
        return false;
    else if (valueTypes[i] == VALUE_STR)
      if (!ai->IsString())
        return false;
    else if (valueTypes[i] == VALUE_ARRAY)
      if (!ai->IsArray())
        return false;
    else if (valueTypes[i] == VALUE_FUNC)
      if (!ai->IsFunction())
        return false;
    else if (valueTypes[i] == VALUE_OBJ)
      if (!ai->IsObject())
        return false;
  }

  return true;
}

void DEBUGError(const char* format, ...)
{
  const int MSG_MAX_LENGTH = 1024;
  const char* MSG_PREFIX = "ERROR (DBG): ";
  char msg[MSG_MAX_LENGTH];

  int cx = snprintf(msg, MSG_MAX_LENGTH, "%s", MSG_PREFIX);
  if (cx < 0)
    ThrowError("ERROR (DBG): received error, couldn't get message (the irony)");

  va_list args;
  va_start(args, format);
  int cx2 = vsnprintf(msg + cx, MSG_MAX_LENGTH - cx, format, args);
  va_end(args);
  if (cx2 < 0)
    ThrowError("ERROR (DBG): received error, couldn't get message (the irony)");

  if ((cx + cx2) >= MSG_MAX_LENGTH)
    printf("WARNING (DBG): error message too long, truncated\n");

  ThrowError(msg);
}

NAN_MODULE_INIT(Init)
{
  Set(target,
    New<v8::String>("ClearParameter").ToLocalChecked(),
    New<v8::FunctionTemplate>(ClearParameter)->GetFunction());
  Set(target,
    New<v8::String>("LoadParameter").ToLocalChecked(),
    New<v8::FunctionTemplate>(LoadParameter)->GetFunction());
  Set(target,
    New<v8::String>("CalcParameter").ToLocalChecked(),
    New<v8::FunctionTemplate>(CalcParameter)->GetFunction());
  Set(target,
    New<v8::String>("GetParamData").ToLocalChecked(),
    New<v8::FunctionTemplate>(GetParamData)->GetFunction());
  Set(target,
    New<v8::String>("Setup").ToLocalChecked(),
    New<v8::FunctionTemplate>(Setup)->GetFunction());
}

NODE_MODULE(main, Init)
