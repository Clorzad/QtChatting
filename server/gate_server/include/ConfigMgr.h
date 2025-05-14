#pragma once
#include <map>
#include <string>

struct SectionInfo {
    std::map<std::string,std::string> section_data_;
    SectionInfo() = default;
    ~SectionInfo() = default;
    SectionInfo(const SectionInfo& sec);
    SectionInfo& operator=(const SectionInfo& src);
    std::string operator[](const std::string& key);
};

class ConfigMgr {
public:
    ConfigMgr();
    SectionInfo operator[](const std::string& section);
    ConfigMgr& operator=(const ConfigMgr& src);
    ConfigMgr(const ConfigMgr& src);

private:
    std::map<std::string,SectionInfo> config_map_;
};