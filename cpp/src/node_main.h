#pragma once

// Reveals procedures handled by Node and V8 to the pure C++ modules.
// This should NOT reveal any Node or V8 information (header files, types, etc).

// Sends a debug/assert error message to Node.
// Arguments are formatted according to C's printf standard.
void DEBUGError(const char* format, ...);

// Sends a debug message to Node.
void DEBUGMsg(const char* format, ...);