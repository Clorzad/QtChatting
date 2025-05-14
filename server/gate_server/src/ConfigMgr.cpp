#include "ConfigMgr.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

SectionInfo::SectionInfo(const SectionInfo& sec)
{
    this->section_data_ = sec.section_data_;
}

SectionInfo& SectionInfo::operator=(const SectionInfo& src)
{
    if (&src != this)
        this->section_data_ = src.section_data_;
    return *this;
}

std::string SectionInfo::operator[](const std::string& key)
{
    if (section_data_.find(key) == section_data_.end())
        return "";
    return section_data_[key];
}

ConfigMgr::ConfigMgr()
{
    auto current_path = boost::filesystem::current_path();
    boost::filesystem::path config_path = current_path / "config.ini";
    std::cout << "config path: " << config_path << std::endl;

    // 使用Boost.PropertyTree来读取INI文件
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);

    // 遍历INI文件中的所有section
    for (const auto& section_pair : pt) {
        const std::string& section_name = section_pair.first;
        const boost::property_tree::ptree& section_tree = section_pair.second;

        // 对于每个section，遍历其所有的key-value对
        std::map<std::string, std::string> section_config;
        for (const auto& key_value_pair : section_tree) {
            const std::string& key = key_value_pair.first;
            const std::string& value = key_value_pair.second.get_value<std::string>();
            section_config[key] = value;
        }
        SectionInfo sectionInfo;
        sectionInfo.section_data_ = section_config;
        // 将section的key-value对保存到config_map中
        config_map_[section_name] = sectionInfo;
    }

    // 输出所有的section和key-value对
    for (const auto& section_entry : config_map_) {
        const std::string& section_name = section_entry.first;
        SectionInfo section_config = section_entry.second;
        std::cout << "[" << section_name << "]" << std::endl;
        for (const auto& key_value_pair : section_config.section_data_) {
            std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
        }
    }
}

SectionInfo ConfigMgr::operator[](const std::string& section)
{
    if (config_map_.find(section) == config_map_.end())
        return SectionInfo();
    return config_map_[section];
}

ConfigMgr& ConfigMgr::operator=(const ConfigMgr& src)
{
    if (&src != this)
        config_map_ = src.config_map_;
    return *this;
}

ConfigMgr::ConfigMgr(const ConfigMgr& src)
{
    config_map_ = src.config_map_;
}
