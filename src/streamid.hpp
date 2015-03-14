#ifndef streamid_h__
#define streamid_h__

#include <cstdint>
union StreamID
{
	uint32_t sock;
	struct
	{
		uint32_t ip;
		uint16_t port;
		uint16_t pad;
	} source;
	uint64_t total;
	StreamID() : total(0){}
	StreamID(uint32_t sock_) : sock(sock_){}
	StreamID(uint32_t ip_, uint16_t port_) : source({ ip_, port_, 0 }){}
	bool operator < (const StreamID &rV) const
	{
		return total < rV.total;
	}
	bool operator == (const StreamID &rV) const
	{
		return total == rV.total;
	}
};
#endif // streamid_h__
