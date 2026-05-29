#ifndef _DIJETANA_CPPBASH_H_
#define _DIJETANA_CPPBASH_H_

#include <string>
#include <vector>

namespace cppBash {
// ----------------------------------
// core exec
// ----------------------------------
int exec(const std::string& cmd, std::string& result);


// ----------------------------------
// helpers
// ----------------------------------
std::vector<std::string> parseLines(const std::string& s);

std::string realpath_str(const std::string& path);


// ----------------------------------
// ls
// ----------------------------------
std::vector<std::string> ls(const std::string& dir,
                                   const std::string& kwargs = "");

// ----------------------------------
// mkdir
// ----------------------------------
void mkdir_p(const std::string& dir, bool p = true);

// ----------------------------------
// cp, mv, rm
// ----------------------------------
void cp(const std::string& src, const std::string& dst, bool recursive = false);

void mv(const std::string& src, const std::string& dst);

void rm(const std::string& target, bool recursive = false, bool force = false);

void touch(const std::string& filename);

void sleep(const int seconds);

void append(const std::string& filename, const std::string& content);
void prepend(const std::string& filename, const std::string& content);
void write(const std::string& filename, const std::string& content);

// ----------------------------------
// find
// ----------------------------------
std::vector<std::string> find(const std::string& root,
                                     const std::string& args = "",
                                     const std::string& name_pattern = "*");

// ----------------------------------
// pwd
// ----------------------------------
std::string pwd();

// --------------------------------
// exists
// --------------------------------
bool exists(const std::string& path);

std::string basename(const std::string& path, bool remove_extension = false);
std::vector<std::string> splitdirs(const std::string& path);
std::string dirname(const std::string& path);


// ----------------------------------
// cat / head / tail
// ----------------------------------
std::vector<std::string> cat(const std::string& filename);

std::vector<std::string> head(const std::string& filename, std::size_t nlines = 10);

std::vector<std::string> tail(const std::string& filename, std::size_t nlines = 10);

// ----------------------------------
// date
// ----------------------------------
std::string date(const std::string& format = "+%Y-%m-%d %H:%M:%S");

// ----------------------------------
// grep
// ----------------------------------
std::vector<std::string> grep(const std::string& filename,
                                     const std::string& needle);

// ----------------------------------
// generic runCommand
// ----------------------------------
std::string runCommand(const std::string& cmd);

// ----------------------------------
// env
// ----------------------------------
void setEnv(const std::string& varname, const std::string& value);

void unsetEnv(const std::string& varname);

std::string getEnv(const std::string& varname);

} // namespace cppBash

#endif // _CPPBASH_H_