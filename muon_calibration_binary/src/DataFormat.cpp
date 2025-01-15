#include "DataFormat.h"

void DataFormat::setName(const char * a)
{
    snprintf(fileName,sizeof(fileName),"%s",a);
}
