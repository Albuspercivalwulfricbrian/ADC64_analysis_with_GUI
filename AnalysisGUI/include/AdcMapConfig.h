#ifndef ADCMAPCONFIG_H
#define ADCMAPCONFIG_H

#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include "json.hpp"

namespace AdcMapConfig
{

    inline std::string findConfigsDir(const std::string &exeDir)
    {
        QStringList candidates = {
            QString::fromStdString(exeDir) + "/configs",
            QString::fromStdString(exeDir) + "/../configs",
            QString::fromStdString(exeDir) + "/../../configs",
            QDir::currentPath() + "/configs",
        };
        for (const auto &c : candidates)
            if (QDir(c).exists())
                return QDir(c).absolutePath().toStdString();
        return {};
    }

    inline std::map<uint32_t, int16_t> load(const std::string &path)
    {
        std::ifstream f(path);
        if (!f.is_open())
            return {};

        nlohmann::json j;
        try
        {
            f >> j;
        }
        catch (...)
        {
            return {};
        }

        std::map<uint32_t, int16_t> out;
        if (j.contains("devices"))
        {
            int16_t idx = 0;
            for (const auto &v : j["devices"])
            {
                uint32_t sn = static_cast<uint32_t>(
                    std::stoul(v.get<std::string>(), nullptr, 16));
                out[sn] = idx++;
            }
        }
        else if (j.contains("adcmap"))
        {
            for (auto &kv : j["adcmap"].items())
                out[static_cast<uint32_t>(std::stoul(kv.key(), nullptr, 16))] = kv.value().get<int16_t>();
        }
        return out;
    }

    inline bool save(const std::string &path, const std::map<uint32_t, int16_t> &m)
    {
        std::vector<uint32_t> ordered(m.size(), 0);
        for (auto &kv : m)
            if (kv.second >= 0 && kv.second < static_cast<int16_t>(m.size()))
                ordered[kv.second] = kv.first;

        nlohmann::json j;
        j["devices"] = nlohmann::json::array();
        for (uint32_t sn : ordered)
        {
            std::ostringstream oss;
            oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << sn;
            j["devices"].push_back(oss.str());
        }

        std::ofstream f(path);
        if (!f.is_open())
            return false;
        f << j.dump(4);
        return true;
    }

} // namespace AdcMapConfig

#endif // ADCMAPCONFIG_H