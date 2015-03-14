#include "sniffer_stream.hpp"


SnifferStream::SnifferStream(int inter, bool mode) :TinsSniffer(inter, mode)
{
	addSnifferCallback(std::bind(&SnifferStream::onCall, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}
