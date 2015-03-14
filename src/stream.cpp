#include "stream.hpp"

Stream::Stream()
{
}

Stream::~Stream()
{
}

void IStreams::addStreamCallback(StreamCallback cb)
{
	m_callbacks.push_back(cb);
}

void Stream::onCall(const StreamID &id, const char *data, size_t size, CallReason reason)
{
	auto &req = m[id].first;
	auto &res = m[id].second;
	switch (reason)
	{
		case CallReasonRequest:
			if (req.complete() || res.begin()) {
				req.reset();
				res.reset();
			}
			req.parse(data, size);
			break;
		case CallReasonResponse:
			if (!req.complete() && res.complete()) {
				req.reset();
				res.reset();
			}
			res.parse(data, size);
			break;
		case CallReasonClose:
			close(id);
			return;
	}
	if (req.complete() && !res.begin()) {
		for (auto &cb : m_callbacks){
			if (cb) {
				cb(id, req.get(), res.get());
			}
		}
	}
	else if (res.complete()) {
		for (auto &cb : m_callbacks){
			if (cb) {
				cb(id, req.get(), res.get());
			}
		}
		req.reset();
		res.reset();
	}
}

void Stream::close(const StreamID &id)
{
	m.erase(id);
}
