#ifndef sniffer_h__
#define sniffer_h__
#include <functional>
#include <vector>
#include <mutex>
#include <list>
#include <tins/tins.h>
#include "type.hpp"

class Sniffer
{
public:
	static Sniffer* New(int interface, bool promiscMode = true);
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void setFilter(const std::string &filter) = 0;
	virtual ~Sniffer();
	void addSnifferCallback(SnifferCallback consumer);
protected:
	std::mutex m_mutex;
	std::list<SnifferCallback> m_callbacks;
	Sniffer();
};

class TinsSniffer : public Sniffer
{
public:
	TinsSniffer(int interface, bool promiscMode);
	virtual ~TinsSniffer();
	virtual void start() override;
	virtual void stop() override;
	virtual void setFilter(const std::string &filter) override;
private:
	void promot(SnifferCallback &cb, const StreamID &id, std::vector<uint8_t> &v, CallReason reason);
	std::unique_ptr<Tins::Sniffer> m_tinsSniffer;
	static const char *defaultFilter;
};

#endif // sniffer_h__
