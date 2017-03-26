// Jose M Rico
// March 26, 2017
// node_main.h

// Reveals procedures handled by Node and V8 to the pure C++ modules.
// This should NOT reveal any Node or V8 information (header files, types, etc).

#pragma once

// Sends a debug/assert error message to Node.
// Arguments are formatted according to C's printf standard.
void DEBUG_error(const char* format, ...);
