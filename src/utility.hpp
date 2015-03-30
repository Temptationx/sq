#ifndef utility_h__
#define utility_h__

#include <string>
#include <exception>

std::string md5(const char *data, size_t size);
int decode_gzip(char *inData, int dataSize, char **outputData, int *outputSize);
class TengineNotCached : public std::exception{};

#endif // utility_h__
