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

### Steps

1. __Install Node.js.__ This process is straightforward. Simply download and run
the installer for your platform at https://nodejs.org/en/. Node should have
installed globally; run "node --version" on your terminal to verify this.

2. __Make sure you have npm installed.__ NPM (Node Package Manager) should also
have been installed globally by the Node.js installer. Run "npm --version" on
your terminal to verify this.

3. __Install node-gyp GLOBALLY.__ The node-gyp package is a command line tool
for building native addon modules for Node.js. This project uses it to build
its native C++ code. Install it globally on your machine by running:

  npm install -g node-gyp
