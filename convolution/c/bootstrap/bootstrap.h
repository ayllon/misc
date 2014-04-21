#ifndef _BOOTSTRAP_H_
#define _BOOTSTRAP_H_

#include "../convolution.h"

void initFilterFromFile(Filter*, const std::string&);
void initImageFromFile(Image*, const std::string&);
void dumpImage(const Image&, const std::string&);

#endif // _BOOTSTRAP_H_
