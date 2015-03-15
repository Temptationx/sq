#ifndef utility_h__
#define utility_h__

#include <string>

std::string md5(const char *data, size_t size);
int decode_gzip(char *inData, int dataSize, char **outputData, int *outputSize);
#endif // utility_h__