// include/tools.h
#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <vector>

// Read all lines from a file into a vector of strings.
// Returns 0 on success, -1 on error (e.g., file not found).
int readLines(const std::string &path, std::vector<std::string> &lines);

// Trim leading/trailing whitespace in-place
void trim(std::string &s);

// Split a line of form "key: value" into key and val (trimmed).
// Returns true if successful, false if no colon found.
bool splitKeyVal(const std::string &line, std::string &key, std::string &val);

// Read a single integral value from a file into out.
// Returns 0 on success, -1 on error.
int readSingleValue(const std::string &path, int &out);

#endif // TOOLS_H