// Jose M Rico
// March 14, 2017
// build.js
// Node script that builds the C++ codebase according to the current platform.

function execCallback(err, stdout, stderr)
{
  if (err)
    console.log(stderr);

  console.log(stderr); // Mac messages get printed here
  console.log(stdout);
}

var exec = require("child_process").exec;

var args = ""
if (process.argv.length > 2 && process.argv[2] === "quick") {
  console.log("Quick build...");
  args = "quick";
}
else {
  console.log("Complete build...");
}

if (process.platform == "win32")
{
  console.log("Building C++ code for Windows");
  exec("cpp\\win_compile_cpp.bat " + args, execCallback);
}
else if (process.platform == "darwin")
{
  console.log("Building C++ code for Mac");
  exec("cpp/mac_compile_cpp.sh " + args, execCallback);
}
else
{
  console.log("ERROR: Unsupported platform");
}