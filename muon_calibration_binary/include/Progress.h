#ifndef PROGRESS_H
#define PROGRESS_H
#include "string.h"
struct Progress
{
  float percentage;
  std::string fileName;
  Progress(const std::string& name) : percentage(0), fileName(name) {}  

};
#endif PROGRESS_H