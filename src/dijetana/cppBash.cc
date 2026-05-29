#include "cppBash.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>


int cppBash::exec(const std::string& cmd, std::string& result)
{
    std::array<char, 256> buffer{};

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        throw std::runtime_error("popen() failed");
    }

    while (fgets(buffer.data(), buffer.size(), pipe))
    {
        result += buffer.data();
    }

    int rc = pclose(pipe);
    return rc;
}

// ----------------------------------
// helpers
// ----------------------------------
std::vector<std::string> cppBash::parseLines(const std::string& s)
{
    std::vector<std::string> out;
    std::istringstream iss(s);
    for (std::string line; std::getline(iss, line);)
    {
        out.push_back(line);
    }
    return out;
}

std::string cppBash::realpath_str(const std::string& path)
{
    std::string cmd = "realpath " + path;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("realpath command failed with return code " + std::to_string(rc));
    }

    auto pos = result.find("/sphenix");
    if (pos != std::string::npos)
    {
        result = result.substr(pos);
    }

    // remove trailing newline
    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}

// ----------------------------------
// ls
// ----------------------------------
std::vector<std::string> cppBash::ls(const std::string& dir,
                                   const std::string& kwargs )
{
    std::string cmd = "ls ";
    if (!kwargs.empty())
    {
        cmd += kwargs + " ";
    }
    cmd += dir;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("ls command failed with return code " + std::to_string(rc));
    }

    auto lines = parseLines(result);
    std::vector<std::string> out;

    for (const auto& line : lines)
    {
        if (line.empty()) continue;
        out.push_back(line);
    }

    return out;
}

// ----------------------------------
// mkdir
// ----------------------------------
void cppBash::mkdir_p(const std::string& dir, bool p )
{
    std::string cmd = "mkdir ";
    if (p)
    {
        cmd += "-p ";
    }
    cmd += dir;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("mkdir command failed with return code " + std::to_string(rc));
    }
}

// ----------------------------------
// cp, mv, rm
// ----------------------------------
void cppBash::cp(const std::string& src, const std::string& dst, bool recursive )
{
    std::string cmd = "cp ";
    if (recursive)
    {
        cmd += "-r ";
    }
    cmd += src + " " + dst;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("cp command failed with return code " + std::to_string(rc));
    }
}

void cppBash::mv(const std::string& src, const std::string& dst)
{
    std::string cmd = "mv " + src + " " + dst;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("mv command failed with return code " + std::to_string(rc));
    }
}

void cppBash::rm(const std::string& target, bool recursive , bool force )
{
    std::string cmd = "rm ";
    if (recursive)
    {
        cmd += "-r ";
    }
    if (force)
    {
        cmd += "-f ";
    }
    cmd += target;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("rm command failed with return code " + std::to_string(rc));
    }
}

// ----------------------------------
// find
// ----------------------------------
std::vector<std::string> cppBash::find(const std::string& root,
                                     const std::string& args,
                                     const std::string& name_pattern)
{
    std::string cmd = "find " + root;
    if (!args.empty())
    {
        cmd += " " + args;
    }
    cmd += " -name \"" + name_pattern + "\"";

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("find command failed with return code " + std::to_string(rc));
    }

    return parseLines(result);
}

// ----------------------------------
// pwd
// ----------------------------------
std::string cppBash::pwd()
{
    std::string cmd = "pwd";

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("pwd command failed with return code " + std::to_string(rc));
    }

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return realpath_str(result);
}

// ----------------------------------
// cat / head / tail
// ----------------------------------
std::vector<std::string> cppBash::cat(const std::string& filename)
{
    std::string cmd = "cat " + filename;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("cat command failed with return code " + std::to_string(rc));
    }

    return parseLines(result);
}

std::vector<std::string> cppBash::head(const std::string& filename, std::size_t nlines)
{
    std::string cmd = "head -n " + std::to_string(nlines) + " " + filename;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("head command failed with return code " + std::to_string(rc));
    }
    return parseLines(result);
}

std::vector<std::string> cppBash::tail(const std::string& filename, std::size_t nlines)
{
    std::string cmd = "tail -n " + std::to_string(nlines) + " " + filename;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("tail command failed with return code " + std::to_string(rc));
    }
    return parseLines(result);
}

void cppBash::touch(const std::string& filename)
{
    std::string cmd = "touch " + filename;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("touch command failed with return code " + std::to_string(rc));
    }
}

void cppBash::sleep(const int seconds)
{
    std::string cmd = "sleep " + std::to_string(seconds);

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("sleep command failed with return code " + std::to_string(rc));
    }
}

void cppBash::append(const std::string& filename, const std::string& content)
{
    std::ofstream ofs(filename, std::ios::app);
    if (!ofs)
    {
        throw std::runtime_error("Failed to open file '" + filename + "' for appending");
    }
    ofs << content;
    ofs.close();
}
void cppBash::prepend(const std::string& filename, const std::string& content)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        throw std::runtime_error("Failed to open file '" + filename + "' for reading");
    }
    std::string existing_content((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());
    ifs.close();

    std::ofstream ofs(filename);
    if (!ofs)
    {
        throw std::runtime_error("Failed to open file '" + filename + "' for writing");
    }
    ofs << content << existing_content;
    ofs.close();
}   
void cppBash::write(const std::string& filename, const std::string& content)
{
    std::ofstream ofs(filename);
    if (!ofs)
    {
        throw std::runtime_error("Failed to open file '" + filename + "' for writing");
    }
    ofs << content;
    ofs.close();
}


// ----------------------------------
// date
// ----------------------------------
std::string cppBash::date(const std::string& format )
{
    std::string cmd = "date '" + format + "'";

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("date command failed with return code " + std::to_string(rc));
    }

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}

// ----------------------------------
// grep
// ----------------------------------
std::vector<std::string> cppBash::grep(const std::string& filename,
                                     const std::string& needle)
{
    std::string cmd = "grep '" + needle + "' " + filename;

    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("grep command failed with return code " + std::to_string(rc));
    }

    return parseLines(result);
}

// ----------------------------------
// generic runCommand
// ----------------------------------
std::string cppBash::runCommand(const std::string& cmd)
{
    std::string result;
    int rc = exec(cmd, result);
    if (rc != 0)
    {
        throw std::runtime_error("command '" + cmd + "' failed with return code " + std::to_string(rc));
    }
    return result;
}

// ----------------------------------
// env
// ----------------------------------
void cppBash::setEnv(const std::string& varname, const std::string& value)
{
    if (setenv(varname.c_str(), value.c_str(), 1) != 0)
    {
        throw std::runtime_error("setenv failed for variable '" + varname + "'");
    }
}

void cppBash::unsetEnv(const std::string& varname)
{
    if (unsetenv(varname.c_str()) != 0)
    {
        throw std::runtime_error("unsetenv failed for variable '" + varname + "'");
    }
}

std::string cppBash::getEnv(const std::string& varname)
{
    const char* val = std::getenv(varname.c_str());
    if (!val)
    {
        throw std::runtime_error("getenv: variable '" + varname + "' not set");
    }
    return std::string(val);
}

bool cppBash::exists(const std::string& path)
{
    return std::filesystem::exists(path);
}

std::string cppBash::basename(const std::string& path, bool remove_extension )
{
    std::filesystem::path p(path);
    std::string base;
    if (remove_extension)
    {
        base = p.stem().string();
    }
    else
    {
        base = p.filename().string();
    }
    return base;
}
std::vector<std::string> cppBash::splitdirs(const std::string& path)
{
    std::filesystem::path p(path);
    std::string realpath = realpath_str(path);
    std::filesystem::path rp(realpath);
    std::vector<std::string> dirs;
    for (const auto& part : rp)
    {
        if (part.string() == "/")
        {
            continue;
        }
        dirs.push_back(part.string());
    }
    return dirs;
}
std::string cppBash::dirname(const std::string& path)
{
    std::filesystem::path p(path);
    std::string dir = p.parent_path().string();
    return realpath_str(dir);
}

