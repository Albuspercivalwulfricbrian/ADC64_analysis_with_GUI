#include "fstream"
#include "iomanip"
#include "iostream"
#include "sstream"
#include "vector"

#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct ChannelConfig {
  vector<vector<vector<int>>> v;
  void Initialize(int a, int b, int c) {
    vector<int> temp;
    vector<vector<int>> temp2d;
    for (int i = 0; i < c; i++)
      temp.push_back(0);
    for (int i = 0; i < b; i++)
      temp2d.push_back(temp);
    for (int i = 0; i < a; i++)
      v.push_back(temp2d);
  }
};

class ChannelMap {
public:
  ChannelMap(){};
  ~ChannelMap(){};

  void ReadConfig(const json &a) {
    delete chconfig;
    chconfig = new ChannelConfig();

    for (auto &[key, val] : a["channels"].items()) {

      if (val["x1"] > x1s)
        x1s = val["x1"];
      if (val["x2"] > x2s)
        x2s = val["x2"];
      if (val["x3"] > x3s)
        x3s = val["x3"];
    }

    chconfig->Initialize(x1s, x2s, x3s + 1);
    for (auto &[key, val] : a["channels"].items()) {
      int x = (int)val["x1"] - 1;
      int y = (int)val["x2"] - 1;
      int z = (int)val["x3"];
      if (chconfig->v[x][y][0] == 0)
        chconfig->v[x][y][0] = val["connector"];
      chconfig->v[x][y][z] = stoi(key);
      cout << x << y << z << endl;
    }
  }

  json SaveConfig() {
    int counter = 1;
    json ChannelInfoArray;
    json ChannelInfo;
    for (int i = 0; i < x1s; i++) {
      for (int j = 0; j < x2s; j++) {
        for (int k = 1; k < x3s + 1; k++) {

          ChannelInfo["channel"] = chconfig->v[i][j][k];
          ChannelInfo["connector"] = chconfig->v[i][j][0];
          ChannelInfo["x3"] = k;
          ChannelInfo["x2"] = j + 1;
          ChannelInfo["x1"] = i + 1;

          ChannelInfoArray[std::to_string(chconfig->v[i][j][k])] = ChannelInfo;
          counter++;
        }
      }
    }

    jMap["channels"] = ChannelInfoArray;

    return jMap;
  }

  void ResetConfig(int x1, int x2, int x3) {
    delete chconfig;
    chconfig = new ChannelConfig();
    chconfig->Initialize(x1, x2, x3);
  }

  int GetX1Size() { return x1s; }
  int GetX2Size() { return x2s; }
  int GetX3Size() { return x3s; }

  ChannelConfig *chconfig = nullptr;

private:
  json jMap;
  int x1s = -100;
  int x2s = -100;
  int x3s = -100;
};
