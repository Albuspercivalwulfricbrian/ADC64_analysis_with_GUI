#include <map>
#include <vector>
#include <tuple>
#include <memory>
#include <fstream>
#include <algorithm>
#include <json.hpp>

using json = nlohmann::json;

struct ChannelData
{
    int32_t ch;
    int32_t iadc;
    int32_t connector;
    int32_t adc_sn;
    std::tuple<int32_t, int32_t, int32_t> num_geo;
    std::tuple<double, double, double> pos_geo;
};

class FHCalMapper
{
private:
    std::map<int32_t, std::map<int32_t, std::shared_ptr<ChannelData>>> by_adc;
    std::map<std::tuple<int32_t, int32_t, int32_t>, std::shared_ptr<ChannelData>> by_geo;
    std::map<std::tuple<double, double, double>, std::shared_ptr<ChannelData>> by_pos;
    std::map<int32_t, std::vector<int32_t>> adc_connectors;

public:
    FHCalMapper(const std::string &json_file_path)
    {
        load_json(json_file_path);
    }

    std::shared_ptr<ChannelData> getChannelInfo(int32_t adc_sn, int32_t channel) const
    {
        auto adc_it = by_adc.find(adc_sn);
        if (adc_it == by_adc.end())
        {
            return nullptr;
        }

        auto ch_it = adc_it->second.find(channel);
        if (ch_it == adc_it->second.end())
        {
            return nullptr;
        }

        return ch_it->second;
    }

    // std::shared_ptr<ChannelData> getByAdcConnectorChannel(int32_t adc_sn, int32_t connector, int32_t channel) const
    // {
    //     auto adc_it = by_adc.find(adc_sn);
    //     if (adc_it == by_adc.end())
    //     {
    //         return nullptr;
    //     }

    //     for (const auto &[ch_num, ch_data] : adc_it->second)
    //     {
    //         if (ch_data->connector == connector && ch_data->ch == channel)
    //         {
    //             return ch_data;
    //         }
    //     }
    //     return nullptr;
    // }

    std::shared_ptr<ChannelData> getByGeo(int32_t x, int32_t y, int32_t z) const
    {
        auto it = by_geo.find(std::make_tuple(x, y, z));
        if (it == by_geo.end())
        {
            return nullptr;
        }
        return it->second;
    }

    std::shared_ptr<ChannelData> getByPos(double x, double y, double z) const
    {
        auto it = by_pos.find(std::make_tuple(x, y, z));
        if (it == by_pos.end())
        {
            return nullptr;
        }
        return it->second;
    }

    std::vector<std::shared_ptr<ChannelData>> filterADCchannels(int32_t adc_sn) const
    {
        std::vector<std::shared_ptr<ChannelData>> result;
        auto it = by_adc.find(adc_sn);
        if (it == by_adc.end())
        {
            return result;
        }

        for (const auto &pair : it->second)
        {
            result.push_back(pair.second);
        }
        return result;
    }

    std::vector<std::shared_ptr<ChannelData>> filterADCConnectorchannels(int32_t adc_sn, int32_t connector) const
    {
        std::vector<std::shared_ptr<ChannelData>> result;
        auto it = by_adc.find(adc_sn);
        if (it == by_adc.end())
        {
            return result;
        }

        for (const auto &[ch_num, ch_data] : it->second)
        {
            if (ch_data->connector == connector)
            {
                result.push_back(ch_data);
            }
        }
        return result;
    }

    std::vector<int32_t> getConnectors(int32_t adc_sn) const
    {
        auto it = adc_connectors.find(adc_sn);
        if (it == adc_connectors.end())
        {
            return {};
        }
        return it->second;
    }

    std::vector<int32_t> getADCSerialList() const
    {
        std::vector<int32_t> result;
        for (const auto &pair : by_adc)
        {
            result.push_back(pair.first);
        }
        return result;
    }

private:
    void load_json(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Can't open file: " + path);
        }

        json data;
        file >> data;

        if (!data.contains("ch_geo"))
        {
            throw std::runtime_error("Shit channel info");
        }

        for (const auto &item : data["ch_geo"])
        {
            auto ch = std::make_shared<ChannelData>();

            ch->ch = item["adc_ch"];
            ch->iadc = item["iadc"];
            ch->connector = item["adc_modi"];
            ch->adc_sn = item["adc_sn"];

            auto geo = item["num_geo"];
            ch->num_geo = std::make_tuple(geo[0], geo[1], geo[2]);

            auto pos = item["pos_geo"];
            ch->pos_geo = std::make_tuple(pos[0], pos[1], pos[2]);

            by_adc[ch->adc_sn][ch->ch] = ch;
            by_geo[ch->num_geo] = ch;
            by_pos[ch->pos_geo] = ch;

            auto &connectors = adc_connectors[ch->adc_sn];
            if (std::find(connectors.begin(), connectors.end(), ch->connector) == connectors.end())
            {
                connectors.push_back(ch->connector);
            }
        }
    }
};