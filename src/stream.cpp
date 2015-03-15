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
		auto request = request_parser.get();
		auto resposne = response_parser.get();
		auto url = request ? request->url : std::string();
		auto pkt = std::make_shared<CachePacket>(url, request, resposne);
		for (auto &cb : m_callbacks){
			if (cb) {
				cb(id, pkt);
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
