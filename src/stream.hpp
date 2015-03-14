#ifndef stream_h__
#define stream_h__

#include <functional>
#include <memory>
#include <list>
#include "type.hpp"
#include "parser.hpp"
union StreamID;


class IStream
{
public:
	virtual ~IStream(){};
	virtual void onCall(const StreamID &id, const char *data, size_t size, CallReason reason) = 0;
	virtual void addStreamCallback(StreamCallback cb);
	std::list<StreamCallback> m_callbacks;
};

class Stream : public IStream
{
public:
	Stream();
	virtual ~Stream();
	virtual void onCall(const StreamID &id, const char *data, size_t size, CallReason reason) override;
private:
	void close(const StreamID &id);
	std::map<StreamID, std::pair<RequestParser, ResponseParser>> m;
};

#endif // stream_h__
