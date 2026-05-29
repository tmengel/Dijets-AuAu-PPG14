#ifndef _DIJETANA_ANACONF_H_
#define _DIJETANA_ANACONF_H_

#include <string>
#include <unordered_map>
#include <vector>

class AnaConf
{
 public:
    
    AnaConf() = default;
    ~AnaConf() = default;

    bool Load(const std::string& filename);

    int GetInt(const std::string& k, int def = 0) const;
    float GetFloat(const std::string& k, float def = 0.0f) const;
    std::string GetString(const std::string& k,
                          const std::string& def = "") const;

    std::unordered_map<std::string, int> ints;
    std::unordered_map<std::string, float> floats;
    std::unordered_map<std::string, std::string> strings;

    std::unordered_map<std::string, std::vector<int>> int_vectors;
    std::unordered_map<std::string, std::vector<float>> float_vectors;
    std::unordered_map<std::string, std::vector<std::string>> string_vectors;

 private:

    static void StripComment(std::string& line);
    static void Trim(std::string& s);

    static std::vector<int> ParseIntVec(const std::string& s);
    static std::vector<float> ParseFloatVec(const std::string& s);
    static std::vector<std::string> ParseStringVec(const std::string& s);
};

#endif //_DIJETANA_ANACONF_H_