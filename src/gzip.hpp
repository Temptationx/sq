#ifndef gzip_h__
#define gzip_h__

int decode_gzip(char *inData, int dataSize, char **outputData, int *outputSize);
#endif // gzip_h__