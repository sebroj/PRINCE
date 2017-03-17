// Jose M Rico
// March 15, 2017
// main.cpp

#include <stdio.h>
#include <vector>

#include <node.h>

#define MAX_LINE_SIZE 512

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

bool load_file_1d(const char* path)
{
  printf("Load 1-D data from %s\n", path);

  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  std::vector<double[2]> data;
  char buf[MAX_LINE_SIZE];
  while (fgets(buf, MAX_LINE_SIZE, fp))
  {
    char* it = buf;
    double lineData[2];
    while (*it != ',')
      it++;

  }
}

bool load_file_2d(const char* path)
{
  printf("Load 2-D data from %s\n", path);

  FILE* fp = fopen(path, "r");
  if (fp == NULL)
    return false;

  std::vector<double[3]> data;
  char buf[MAX_LINE_SIZE];
  while (fgets(buf, MAX_LINE_SIZE, fp))
  {
  }
}

const char* to_c_string(const String::Utf8Value& value)
{
  return *value ? *value : "STRING CONVERSION FAILED";
}

void load_file(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 2)
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Expected 2 args.")));
    return;
  }

  if (!args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Argument 1 should be string.")));
    return;
  }
  if (!args[1]->IsInt32())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Argument 2 should be integer.")));
    return;
  }

  String::Utf8Value string(args[0]);
  const char* cstring = to_c_string(string);
  int dataDim = args[1]->Int32Value();

  if (dataDim == 1)
    load_file_1d(cstring);
  else
    load_file_2d(cstring);

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, cstring));
}

void init(Local<Object> exports)
{
  NODE_SET_METHOD(exports, "load_file", load_file);
}

NODE_MODULE(addon, init)
