// Jose M Rico
// March 12, 2017
// scary.cpp

#include <node.h>

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

const char* to_c_string(const String::Utf8Value& value)
{
  return *value ? *value : "STRING CONVERSION FAILED";
}

char* intense(int* n)
{
  for (int it = 0; it < 2; it++)
  {
    int N = *n;
    float* floats = new float[N];

    int fact = 1;
    for (int i = 0; i < N; i++)
    {
      fact += fact / N * i + i - 10;
      floats[i] = (float)fact + 1;
    }

    delete[] floats;
  }

  return "DONE";
}

void procedure(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Expected one argument.")));
    return;
  }

  if (!args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Argument should be string.")));
    return;
  }

  String::Utf8Value string(args[0]);
  const char* cstr = to_c_string(string);

  int n = 100000000;
  char* output = intense(&n);

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, cstr));
}

void init(Local<Object> exports)
{
  NODE_SET_METHOD(exports, "execute", procedure);
}

NODE_MODULE(addon, init)
