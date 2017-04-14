// Jose M Rico
// March 15, 2017
// node_main.cpp

// Main entry point for the Node.js addon interface.
// Only this file should know anything about Node and V8.

#include "node_main.h"

// TODO port to nan for compatibility and robustness.
//#include <nan.h>
#include <node.h>

#include <cstdio>
#include <stdarg.h>
#include <vector>

#include "load_data.h"

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

enum ArgTypes
{
  ARG_INT,
  ARG_STR,
  ARG_ARRAY
};

class StringV8
{
  String::Utf8Value str;

public:
  StringV8(const Local<Value>& value)
    : str(value)
  {}

  const char* cstr() const
  {
    return *str ? *str : "STRING CONVERSION FAILED";
  }
};

static bool func_verify_args(
  const char* funcName,
  const FunctionCallbackInfo<Value>& args,
  const std::vector<ArgTypes>& argTypes);

static void clear_parameter(const FunctionCallbackInfo<Value>& args)
{
  std::vector<ArgTypes> argTypes({ARG_STR});
  if (!func_verify_args("clear_parameter", args, argTypes))
    return;

  StringV8 alias(args[0]);
  clear_data(alias.cstr());
}

static void load_file(const FunctionCallbackInfo<Value>& args)
{
  std::vector<ArgTypes> argTypes({ARG_STR, ARG_STR, ARG_INT, ARG_ARRAY});
  if (!func_verify_args("load_file", args, argTypes))
    return;

  // Verify array argument
  Local<v8::Array> array = Local<v8::Array>::Cast(args[3]);
  if (array->Length() != 2)
  {
    DEBUG_error("load_file coordTypes array length is %d, but should be 2",
      array->Length());
    return;
  }
  if (!array->Get(0)->IsInt32() || !array->Get(1)->IsInt32())
  {
    DEBUG_error("load_file coordTypes array elements should be integers");
    return;
  }
  int a0 = array->Get(0)->Int32Value();
  int a1 = array->Get(1)->Int32Value();
  if (a0 < COORD_NONE || a0 > COORD_Z || a1 < COORD_NONE || a1 > COORD_Z)
  {
    DEBUG_error("load_file coordTypes array elements should be CoordType values");
    return;
  }

  // Translate arguments for load procedure.
  StringV8 filePath(args[0]);
  StringV8 alias(args[1]);
  int dataDim = args[2]->Int32Value();
  CoordType coordTypes[2] = { (CoordType)a0, (CoordType)a1 };

  bool success = load_data(filePath.cstr(), alias.cstr(), dataDim, coordTypes);
  args.GetReturnValue().Set(success);
}

static void get_param_data(const FunctionCallbackInfo<Value>& args)
{
  std::vector<ArgTypes> argTypes({ARG_STR});
  if (!func_verify_args("get_param_data", args, argTypes))
    return;

  StringV8 alias(args[0]);

  // Attempt to retrieve the data.
  Isolate* isolate = args.GetIsolate();
  const std::vector<double>* valuesPtr = get_values(alias.cstr());
  if (valuesPtr == nullptr)
  {
    args.GetReturnValue().Set(v8::Null(isolate));
    return;
  }
  const std::vector<std::vector<double>>& points = *get_points(alias.cstr());
  const std::vector<double>& values = *valuesPtr;
  int dataDim = (int)points[0].size();

  Local<Object> result = v8::Object::New(isolate);

  Local<String> dimString = v8::String::NewFromUtf8(isolate, "dim");
  result->Set(dimString, v8::Number::New(isolate, dataDim));

  if (dataDim == 1)
  {
    // Transfer 1-D data to the V8 runtime.
    Local<v8::Array> data = v8::Array::New(isolate, (int)points.size() * 2);
    for (int i = 0; i < (int)points.size(); i++)
    {
      Local<v8::Number> val = v8::Number::New(isolate, points[i][0]);
      data->Set(i, val);
    }
    for (int i = 0; i < (int)values.size(); i++)
    {
      Local<v8::Number> val = v8::Number::New(isolate, values[i]);
      data->Set((int)points.size() + i, val);
    }

    Local<String> dataString = v8::String::NewFromUtf8(isolate, "data");
    result->Set(dataString, data);
  }
  if (dataDim == 2)
  {
    int height = 0;
    double firstX = points[0][0];
    while (points[++height][0] == firstX) {}
    int width = (int)values.size() / height;
    Local<String> widthString = v8::String::NewFromUtf8(isolate, "width");
    result->Set(widthString, v8::Number::New(isolate, width));
    Local<String> heightString = v8::String::NewFromUtf8(isolate, "height");
    result->Set(heightString, v8::Number::New(isolate, height));
    // Transfer 2-D data to the V8 runtime.
    std::vector<double> valuesRegular;
    to_regular_grid(points, values, valuesRegular);
    Local<v8::Array> data = v8::Array::New(isolate, (int)valuesRegular.size());
    for (int i = 0; i < (int)valuesRegular.size(); i++)
    {
      Local<v8::Number> val = v8::Number::New(isolate, values[i]);
      // Change index from column-grouping to row-grouping.
      int row = i % height;
      int col = i / height;
      data->Set(row * width + col, val);
    }

    Local<String> dataString = v8::String::NewFromUtf8(isolate, "data");
    result->Set(dataString, data);
  }

  args.GetReturnValue().Set(result);
}

static void setup_parameters(const FunctionCallbackInfo<Value>& args)
{
  std::vector<ArgTypes> argTypes({ARG_INT});
  if (!func_verify_args("setup_parameters", args, argTypes))
    return;

  // TODO possibly pass parameter names? or at least symbols for equations...

  int paramCount = args[0]->Int32Value();
  set_parameter_count(paramCount);
}

void DEBUG_error(const char* format, ...)
{
  const int MSG_MAX_LENGTH = 1024;
  const char* MSG_PREFIX = "ERROR (DBG): ";
  Isolate* isolate = Isolate::GetCurrent();
  char msg[MSG_MAX_LENGTH];

  int cx = snprintf(msg, MSG_MAX_LENGTH, "%s", MSG_PREFIX);
  if (cx < 0)
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,
      "ERROR (DBG): received error, failed to parse message (oh the irony)")));

  va_list args;
  va_start(args, format);
  int cx2 = vsnprintf(msg + cx, MSG_MAX_LENGTH - cx, format, args);
  va_end(args);
  if (cx2 < 0)
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,
      "ERROR (DBG): received error, failed to parse message (oh the irony)")));

  if ((cx + cx2) >= MSG_MAX_LENGTH)
    printf("WARNING (DBG): error message too long, truncated\n");

  isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, msg)));
}

static bool func_verify_args(
  const char* funcName,
  const FunctionCallbackInfo<Value>& args,
  const std::vector<ArgTypes>& argTypes)
{
  if (args.Length() != (int)argTypes.size())
  {
    DEBUG_error("%s expected %d arguments", funcName, (int)argTypes.size());
    return false;
  }
  for (int i = 0; i < args.Length(); i++)
  {
    if (argTypes[i] == ARG_INT)
    {
      if (!args[i]->IsInt32())
      {
        DEBUG_error("%s arg %d should be an int", funcName, i+1);
        return false;
      }
    }
    else if (argTypes[i] == ARG_STR)
    {
      if (!args[i]->IsString())
      {
        DEBUG_error("%s arg %d should be a string", funcName, i+1);
        return false;
      }
    }
    else if (argTypes[i] == ARG_ARRAY)
    {
      if (!args[i]->IsArray())
      {
        DEBUG_error("%s arg %d should be an array", funcName, i+1);
        return false;
      }
    }
  }

  return true;
}

static void init(Local<Object> exports)
{
  NODE_SET_METHOD(exports, "clear_parameter", clear_parameter);
  NODE_SET_METHOD(exports, "load_file", load_file);
  NODE_SET_METHOD(exports, "setup_parameters", setup_parameters);
  NODE_SET_METHOD(exports, "get_param_data", get_param_data);
}

NODE_MODULE(main, init)
