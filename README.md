# PRINCE
The Plasma Rocket Instability Characterizer (PRINCE) is a software tool capable
of conducting robust and customizable numerical characterizations of waves and
instabilities in plasma thrusters. It currently exists as a prototype,
implemented in the Wolfram Mathematica platform. This project will be a complete
translation, enhancement, and optimization of PRINCE, with all its core
procedures and mathematical algorithms written in native C++ code. This will
ensure that the new PRINCE is not only a coherent and accessible software tool,
but that it uses the available hardware intelligently to be as efficient as
possible across a wide range of platforms.

## Build Instructions
PRINCE uses the Electron framework for its GUI. Though it is a standalone
application, it requires specific tools for building and testing.

### Prerequisites

1. __Install Node.js.__ This process is straightforward. Simply download and run
the installer for your platform at https://nodejs.org/en/. Node should have
installed globally; run the following command on your terminal to verify this:
```
node --version
```

2. __Install NPM.__ NPM (Node Package Manager) should have been installed
globally by the Node.js installer. Check its version on your terminal to verify
this:
```
npm --version
```

3. __Install node-gyp GLOBALLY.__ The node-gyp package is a command line tool
for building native addon modules for Node.js. This project uses it to build
its native C++ code. You can install it globally through NPM on your machine.
However, depending on your platform, you need to make sure other dependencies
are installed, such as Python 2.7 and a C++ compiler. Refer to the
"Installation" section of https://github.com/nodejs/node-gyp for detailed
instructions.

### Build steps

1. __Clone or download this repository.__

2. __Install Node modules.__ Navigate to the root directory of this project
(the one with this README file), and run:
```
npm install
```
This will read the dependencies listed in the package.json file and download
them to the current directory.

3. __Build C++ code.__ From the root directory, run
```
npm run build-cpp
```
This will execute the C++ build scripts for your platform and compile the C++
code into a Node addon.

4. __Test the application.__ From the root directory, run
```
npm start
```
This will start the application.
