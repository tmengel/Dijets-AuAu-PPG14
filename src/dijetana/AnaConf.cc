#include "AnaConf.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

bool AnaConf::Load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open: " << filename << "\n";
        return false;
    }

    std::string line;

    while (std::getline(file, line))
    {
        StripComment(line);
        Trim(line);
        if (line.empty())
        {
            continue;
        } 

        std::istringstream iss(line);

        std::string type, key;
        if (!(iss >> type >> key))
        {
            continue;
        }
        std::string value;
        std::getline(iss, value);
        Trim(value);

        if (type == "int")
        {
            ints[key] = std::stoi(value);
        }

        else if (type == "float"){
            floats[key] = std::stof(value);}

        else if (type == "string"){
            strings[key] = value;}

        else if (type == "vint"){
            int_vectors[key] = ParseIntVec(value);}

        else if (type == "vfloat"){
            float_vectors[key] = ParseFloatVec(value);}

        else if (type == "vstring"){
            string_vectors[key] = ParseStringVec(value);}
    }

    return true;
}

int AnaConf::GetInt(const std::string& k, int def) const
{
    auto it = ints.find(k);
    return (it != ints.end()) ? it->second : def;
}

float AnaConf::GetFloat(const std::string& k, float def) const
{
    auto it = floats.find(k);
    return (it != floats.end()) ? it->second : def;
}

std::string AnaConf::GetString(const std::string& k,
                               const std::string& def) const
{
    auto it = strings.find(k);
    return (it != strings.end()) ? it->second : def;
}

void AnaConf::StripComment(std::string& line)
{
    auto pos = line.find('#');
    if (pos != std::string::npos){
        line = line.substr(0, pos);}
}

void AnaConf::Trim(std::string& s)
{
    auto not_space = [](unsigned char c) { return !std::isspace(c); };

    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), not_space));

    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(),
            s.end());
}

std::vector<int> AnaConf::ParseIntVec(const std::string& s)
{
    std::vector<int> out;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        Trim(item);
        if (!item.empty()){ out.push_back(std::stoi(item));}
    }
    return out;
}

std::vector<float> AnaConf::ParseFloatVec(const std::string& s)
{
    std::vector<float> out;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        Trim(item);
        if (!item.empty()){
            out.push_back(std::stof(item));}
    }
    return out;
}

std::vector<std::string> AnaConf::ParseStringVec(const std::string& s)
{
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        Trim(item);
        if (!item.empty()){
            out.push_back(item);}
    }
    return out;
}