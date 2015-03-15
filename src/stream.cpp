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
	auto &request_parser = m[id].first;
	auto &response_parser = m[id].second;
	switch (reason)
	{
		case CallReasonRequest:
			if (request_parser.complete() || response_parser.begin()) {
				request_parser.reset();
				response_parser.reset();
			}
			request_parser.parse(data, size);
			break;
		case CallReasonResponse:
			if (!request_parser.complete() && response_parser.complete()) {
				request_parser.reset();
				response_parser.reset();
			}
			response_parser.parse(data, size);
			break;
		case CallReasonClose:
			close(id);
			return;
	}
	if (request_parser.complete() || response_parser.complete()) {
		for (auto &cb : m_callbacks){
			if (cb) {
				cb(id, std::make_shared<CachePacket>(request_parser.get()->url,
					request_parser.get(),
					response_parser.get()));
			}
		}
	}
	if (response_parser.complete()) {
		request_parser.reset();
		response_parser.reset();
	}
}

void Stream::close(const StreamID &id)
{
	m.erase(id);
}
