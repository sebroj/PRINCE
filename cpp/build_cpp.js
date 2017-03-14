// Jose M Rico
// March 14, 2017
// build.js
// Node script that builds the C++ codebase according to the current platform.

if (process.platform == "win32")
{
  console.log("Building C++ code for Windows");
  var exec = require("child_process").exec;
  exec("win_build_cpp.bat", function(err, stdout, stderr)
  {
    if (err)
      console.log(stderr);

    console.log(stdout);
  });
}

if (process.platform == "darwin")
{
  console.log("Building C++ code for Mac");
  console.log("OOPS! Unimplemented...");
}
