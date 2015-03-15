#include <md5.h>
#include <string>

#include <zlib.h>
#include <stdlib.h>
#include <memory.h>

#ifdef _WIN32
#	define MEMCPY(__DST__, __SIZE__, __SRC__) memcpy_s(__DST__, __SIZE__, __SRC__, __SIZE__);
#else
#	define MEMCPY(__DST__, __SIZE__, __SRC__) memcpy(__DST__, __SRC__, __SIZE__);
#endif // _WIN32

void charHex(unsigned char c, unsigned char &l, unsigned char &h)
{
	l = c & 0x0F;
	h = c >> 4;

	if (l > 9) {
		l = 'A' + l - 10;
	}
	else{
		l = '0' + l;
	}

	if (h > 9) {
		h = 'A' + h - 10;
	}
	else{
		h = '0' + h;
	}
}

std::string md5(const char *data, size_t size)
{
	MD5_CTX ctx = { 0 };
	MD5_Init(&ctx);
	MD5_Update(&ctx, data, size);
	unsigned char result[16];
	MD5_Final(result, &ctx);
	char hex[32];
	for (int i = 0; i < 16; i++)
	{
		unsigned char l, h;
		charHex(result[i], l, h);
		hex[i * 2] = h;
		hex[i * 2 + 1] = l;
	}
	std::string hex_str = std::string(hex, 32);
	return hex_str;
}

struct Node
{
	Node *next;
	void *data;
};

struct LinkList
{
	Node *first;
	Node *last;
};

LinkList *Create()
{
	LinkList *_list = (LinkList*)malloc(sizeof(LinkList));
	if (!_list) {
		return 0;
	}
	memset(_list, 0, sizeof(LinkList));
	return _list;
}

void Destory(LinkList *_list)
{
	if (!_list) {
		return;
	}
	Node *node = _list->first;
	while (node)
	{
		Node *next = node->next;
		free(node);
		node = next;
	}
	free(_list);
}

void DestoryWithData(LinkList *_list, void(*DeleteFunc)(void*))
{
	if (!_list) {
		return;
	}
	Node *node = _list->first;
	while (node)
	{
		Node *next = node->next;
		if (DeleteFunc) {
			DeleteFunc(next->data);
		}
		free(node);
		node = next;
	}
	//free(_list);
}


void push_back(LinkList *_list, void *data)
{
	if (!_list || !data) {
		return;
	}
	Node *node = (Node*)malloc(sizeof(Node));
	node->next = 0;
	node->data = data;
	if (!_list->first) {
		_list->first = node;
	}
	if (!_list->last) {
		_list->last = node;
	}
	else{
		_list->last->next = node;
		_list->last = node;
	}
}

Node* GetNext(LinkList *_list, Node *node)
{
	if (!node) {
		return _list->first;
	}
	return node->next;
}

struct Data
{
	void *data;
	int size;
};

void DeleteData(void *p)
{
	free(((Data*)p)->data);
	free(p);
}

int decode_gzip(char *inData, int dataSize, char **outputData, int *outputSize)
{
	if (!outputData || !outputSize) {
		return 0;
	}
	*outputSize = 0;
	*outputData = 0;
	z_stream strm = { 0 };
	int ret = inflateInit2(&strm, 15 + 32);
	if (ret != Z_OK)
	{
		inflateEnd(&strm);
		return 0;
	}
	LinkList _list = { 0 };
	strm.avail_in = dataSize;
	strm.next_in = (Bytef*)inData;
	bool anted = false;
	int total_size = 0;
	do
	{
		int outSize = strm.avail_in * 3;
		char *outData = (char*)malloc(outSize);
		if (!outData) {
			DestoryWithData(&_list, DeleteData);
			inflateEnd(&strm);
			return 0;
		}
		strm.avail_out = outSize;
		strm.next_out = (Bytef *)outData;
		ret = inflate(&strm, Z_NO_FLUSH);
		if (ret == Z_DATA_ERROR && !anted) {
			anted = true;
			inflateEnd(&strm);
			strm = { 0 };
			ret = inflateInit2(&strm, -15);
			if (ret != Z_OK) {
				free(outData);
				return 0;
			}
			else{
				strm.avail_in = dataSize;
				strm.next_in = (Bytef*)inData;
			}
		}
		else if (ret < 0 || ret == Z_NEED_DICT) {
			DestoryWithData(&_list, DeleteData);
			inflateEnd(&strm);
			free(outData);
			return 0;
		}
		else{
			int deSize = outSize - strm.avail_out;
			total_size += deSize;

			Data *data_ = (Data*)malloc(sizeof(Data));
			data_->data = outData;
			data_->size = deSize;
			push_back(&_list, data_);
		}

	} while (strm.avail_in > 0);
	inflateEnd(&strm);
	char *outData = (char*)malloc(total_size);
	int pos = 0;

	Node *node = _list.first;
	while (node)
	{
		MEMCPY(outData + pos, ((Data*)(node->data))->size, ((Data*)(node->data))->data);
		pos += ((Data*)(node->data))->size;
		Node *next = node->next;
		free(((Data*)(node->data))->data);
		free(node->data);
		free(node);
		node = next;
	}

	*outputSize = total_size;
	*outputData = outData;
	return 1;
}