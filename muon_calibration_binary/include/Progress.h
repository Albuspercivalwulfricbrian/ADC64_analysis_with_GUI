#ifndef PROGRESS_H
#define PROGRESS_H
#include "string.h"
struct Progress
{
  float percentage;
  std::string fileName;
  bool active;
  Progress(const std::string& name) : percentage(0), fileName(name), active(false) {}  

};
#endif PROGRESS_H