#ifndef sniffer_h__
#define sniffer_h__
#include <functional>
#include <vector>
#include <mutex>
#include <list>
#include <tins/tins.h>
#include "type.hpp"


class ISniffer
{
public:
	typedef std::function<void(const StreamID &, const char*, size_t, CallReason)> SnifferCallback;
	ISniffer();
	virtual void startSniff() = 0;
	virtual void stopSniff() = 0;
	virtual void setFilter(const std::string &filter) = 0;
	virtual ~ISniffer();
	void addSnifferCallback(SnifferCallback consumer);
protected:
	std::mutex m_mutex;
	std::list<SnifferCallback> m_callbacks;
};

class TinsSniffer : public ISniffer
{
public:
	TinsSniffer(int interface, bool promiscMode);
	virtual ~TinsSniffer();
	virtual void startSniff() override;
	virtual void stopSniff() override;
	virtual void setFilter(const std::string &filter) override;
private:
	void onData(Tins::TCPStream &st) const;
	void onClose(Tins::TCPStream &st) const;
	void callCallback(const SnifferCallback &cb, const StreamID &id, std::vector<uint8_t> &v, CallReason reason) const;
	std::unique_ptr<Tins::Sniffer> m_tinsSniffer;
	static const char *defaultFilter;
};

#endif // sniffer_h__
