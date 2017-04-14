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

enum ValueTypes
{
  VALUE_INT,
  VALUE_STR,
  VALUE_ARRAY
};

static bool verify_cb_info(
  const char* funcName,
  const FunctionCallbackInfo<v8::Value>& cbInfo,
  const std::vector<ValueTypes>& valueTypes);
static bool verify_array(
  const v8::Local<v8::Array>& array,
  const std::vector<ValueTypes>& valueTypes);

class StringConv
{
  const Utf8String str;

public:
  StringConv(const v8::Local<v8::Value>& value)
    : str(value)
  {}

  const char* cstr() const
  {
    return *str ? *str : "STRING CONVERSION FAILED";
  }
};

NAN_METHOD(clear_parameter)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes({ VALUE_STR });
  if (!verify_cb_info("clear_parameter", cbInfo, cbInfoTypes))
    return;

  StringConv alias(cbInfo[0]);
  clear_data(alias.cstr());
}

NAN_METHOD(load_file)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes(
    { VALUE_STR, VALUE_STR, VALUE_INT, VALUE_ARRAY });
  if (!verify_cb_info("load_file", cbInfo, cbInfoTypes))
    return;

  // Verify array argument
  v8::Local<v8::Array> array = cbInfo[3].As<v8::Array>();
  std::vector<ValueTypes> arrayTypes({ VALUE_INT, VALUE_INT });
  if (!verify_array(array, arrayTypes))
  {
    DEBUG_error("load_file coordTypes array is misformatted.");
    return;
  }
  int a0 = Nan::To<int>(Nan::Get(array, 0).ToLocalChecked()).FromJust();
  int a1 = Nan::To<int>(Nan::Get(array, 1).ToLocalChecked()).FromJust();
  if (a0 < COORD_NONE || a0 > COORD_Z || a1 < COORD_NONE || a1 > COORD_Z)
  {
    DEBUG_error("load_file coordTypes array elements should be CoordType values");
    return;
  }

  // Translate arguments for load procedure.
  StringConv filePath(cbInfo[0]);
  StringConv alias(cbInfo[1]);
  int dataDim = Nan::To<int>(cbInfo[2]).FromJust();
  CoordType coordTypes[2] = { (CoordType)a0, (CoordType)a1 };

  bool success = load_data(filePath.cstr(), alias.cstr(), dataDim, coordTypes);
  cbInfo.GetReturnValue().Set(success);
}

NAN_METHOD(get_param_data)
{
  const FunctionCallbackInfo<v8::Value> &cbInfo = info;
  std::vector<ValueTypes> cbInfoTypes({ VALUE_STR });
  if (!verify_cb_info("get_param_data", cbInfo, cbInfoTypes))
    return;

  StringConv alias(cbInfo[0]);

  // Attempt to retrieve the data.
  const std::vector<double>* valuesPtr = get_values(alias.cstr());
  if (valuesPtr == nullptr)
  {
    cbInfo.GetReturnValue().Set(Null());
    return;
  }
  const std::vector<std::vector<double>>& points = *get_points(alias.cstr());
  const std::vector<double>& values = *valuesPtr;
  int dataDim = (int)points[0].size();

  v8::Local<v8::Object> result = New<v8::Object>();

  Set(result, New<v8::String>("dim").ToLocalChecked(), New<v8::Int32>(dataDim));

  if (dataDim == 1)
  {
    // Transfer 1-D data to the V8 runtime.
    v8::Local<v8::Array> data = New<v8::Array>();
    for (int i = 0; i < (int)points.size(); i++)
      Set(data, i, New<v8::Number>(points[i][0]));

    for (int i = 0; i < (int)values.size(); i++)
      Set(data, (int)points.size() + i, New<v8::Number>(values[i]));

    Set(result, New<v8::String>("data").ToLocalChecked(), data);
  }
  if (dataDim == 2)
  {
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
    v8::Local<v8::Array> data = New<v8::Array>();
    for (int i = 0; i < (int)valuesRegular.size(); i++)
    {
      // Change index from column-grouping to row-grouping.
      int row = i % height;
      int col = i / height;
      Set(data, row * width + col, New<v8::Number>(values[i]));
    }

    Set(result, New<v8::String>("data").ToLocalChecked(), data);
  }

  cbInfo.GetReturnValue().Set(result);
}

static bool verify_cb_info(
  const char* funcName,
  const FunctionCallbackInfo<v8::Value>& cbInfo,
  const std::vector<ValueTypes>& valueTypes)
{
  if (cbInfo.Length() != (int)valueTypes.size())
  {
    DEBUG_error("%s expected %d arguments", funcName, (int)valueTypes.size());
    return false;
  }
  // TODO this possibly still uses non-NAN stuff.
  for (int i = 0; i < cbInfo.Length(); i++)
  {
    if (valueTypes[i] == VALUE_INT)
    {
      if (!cbInfo[i]->IsInt32())
      {
        DEBUG_error("%s arg %d should be an int", funcName, i+1);
        return false;
      }
    }
    else if (valueTypes[i] == VALUE_STR)
    {
      if (!cbInfo[i]->IsString())
      {
        DEBUG_error("%s arg %d should be a string", funcName, i+1);
        return false;
      }
    }
    else if (valueTypes[i] == VALUE_ARRAY)
    {
      if (!cbInfo[i]->IsArray())
      {
        DEBUG_error("%s arg %d should be an array", funcName, i+1);
        return false;
      }
    }
  }

  return true;
}

static bool verify_array(
  const v8::Local<v8::Array>& array,
  const std::vector<ValueTypes>& valueTypes)
{
  if (array->Length() != (int)valueTypes.size())
    return false;

  // TODO this possibly still uses non-NAN stuff.
  for (int i = 0; i < (int)array->Length(); i++)
  {
    v8::Local<v8::Value> ai = Nan::Get(array, i).ToLocalChecked();
    if (valueTypes[i] == VALUE_INT)
      if (!ai->IsInt32())
        return false;

    else if (valueTypes[i] == VALUE_STR)
      if (!ai->IsString())
        return false;

    else if (valueTypes[i] == VALUE_ARRAY)
      if (!ai->IsArray())
        return false;
  }

  return true;
}

void DEBUG_error(const char* format, ...)
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

NAN_MODULE_INIT(init)
{
  Set(target,
    New<v8::String>("clear_parameter").ToLocalChecked(),
    New<v8::FunctionTemplate>(clear_parameter)->GetFunction());
  Set(target,
    New<v8::String>("load_file").ToLocalChecked(),
    New<v8::FunctionTemplate>(load_file)->GetFunction());
  Set(target,
    New<v8::String>("get_param_data").ToLocalChecked(),
    New<v8::FunctionTemplate>(get_param_data)->GetFunction());
}

NODE_MODULE(main, init)
