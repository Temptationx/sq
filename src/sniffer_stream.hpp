#include "../src/stream.hpp"
#include "../src/sniffer.hpp"

class SnifferStream : public TinsSniffer, public Stream
{
public:
	SnifferStream(int inter, bool mode = true);
};
