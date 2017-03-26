// Jose M Rico
// March 15, 2017
// main.cpp

// Main entry point for the Node.js addon interface.
// Only this file should know anything about the Node and the V8 runtime.

#include <node.h>

#include "load_data.h"

// TODO port to nan for compatibility and robustness.
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

static const char* to_c_string(const String::Utf8Value& value)
{
  return *value ? *value : "STRING CONVERSION FAILED";
}

void load_file(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();

  // Check for valid arguments.
  if (args.Length() != 3)
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Expected 3 args.")));
    return;
  }
  if (!args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Arg 1 should be a string (path).")));
    return;
  }
  if (!args[1]->IsInt32())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Arg 2 should be an integer (dimension).")));
    return;
  }
  if (!args[2]->IsInt32())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Arg 3 should be an integer (data ID).")));
    return;
  }

  // Translate arguments to C++ types.
  String::Utf8Value string(args[0]);
  const char* cstring = to_c_string(string);
  int dataDim = args[1]->Int32Value();
  int paramID = args[2]->Int32Value();

  if (!load_data(cstring, dataDim, paramID))
  {
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "FAILED"));
  }
  else
  {
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "Success"));
  }
}

void init(Local<Object> exports)
{
  NODE_SET_METHOD(exports, "load_file", load_file);
}

NODE_MODULE(main, init)
