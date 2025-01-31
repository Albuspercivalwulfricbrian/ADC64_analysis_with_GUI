#ifndef PROGRESS_H
#define PROGRESS_H
#include "string.h"
struct Progress
{
  float percentage;
  std::string fileName;
  bool active;
  bool processed;
  Progress(const std::string& name) : percentage(0), fileName(name), active(false), processed(0) {}  

};
#endif PROGRESS_H