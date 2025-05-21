#pragma once
#include <map>
#include <string>
#include "Singleton.hpp"

struct SectionInfo {
    std::map<std::string,std::string> section_data_;
    SectionInfo() = default;
    ~SectionInfo() = default;
    SectionInfo(const SectionInfo& sec);
    SectionInfo& operator=(const SectionInfo& src);
    std::string operator[](const std::string& key);
};

class ConfigMgr : public Singleton<ConfigMgr> {
    friend class Singleton<ConfigMgr>;
public:
    SectionInfo operator[](const std::string& section);
    ConfigMgr& operator=(const ConfigMgr& src) = delete;
    ConfigMgr(const ConfigMgr& src) = delete;

private:
    ConfigMgr();
    std::map<std::string,SectionInfo> config_map_;
};