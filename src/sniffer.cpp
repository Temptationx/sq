#include "sniffer.hpp"
#include <cassert>

ISniffer::ISniffer()
{
}

ISniffer::~ISniffer()
{
}

void ISniffer::addSnifferCallback(SnifferCallback consumer)
{
	m_callbacks.push_back(consumer);
}

StreamID convertID(const Tins::TCPStream::StreamInfo &info)
{
	return StreamID(info.client_addr, info.client_port);
}

TinsSniffer::TinsSniffer(int inter, bool promiscMode)
{
	Tins::SnifferConfiguration config;
	config.set_buffer_size(1024 * 1024 * 50);
	config.set_promisc_mode(promiscMode);
	auto x=Tins::NetworkInterface::all();
	for (auto n : x){
		printf("%s\n", n.name().data());
		printf("%s\n",n.addresses().ip_addr.to_string().data());
	}
	m_tinsSniffer = std::make_unique<Tins::Sniffer>(Tins::NetworkInterface::all().at(inter).name(), config);
	m_tinsSniffer->set_filter(defaultFilter);
}

TinsSniffer::~TinsSniffer()
{
}

void TinsSniffer::startSniff()
{
	Tins::TCPStreamFollower follower;
	try {
		std::lock_guard<std::mutex> l(m_mutex);

		using tins_fun = std::function<void(Tins::TCPStream &)>;
		tins_fun data_fun = std::bind(&TinsSniffer::onData, this, std::placeholders::_1);
		tins_fun close_fun = std::bind(&TinsSniffer::onClose, this, std::placeholders::_1);

		follower.follow_streams(*m_tinsSniffer, data_fun, close_fun);
	}
	catch (...) {
		printf("sniffer exception");
	}
}

void TinsSniffer::stopSniff()
{
	pcap_breakloop(m_tinsSniffer->get_pcap_handle());
	std::lock_guard<std::mutex> l(m_mutex);
}

void TinsSniffer::setFilter(const std::string &filter)
{
	m_tinsSniffer->set_filter(filter);
}

void TinsSniffer::callCallback(const SnifferCallback &cb, const StreamID &id, std::vector<uint8_t> &v, CallReason reason) const
{
	if (!v.empty()) {
		cb(id, (const char*)v.data(), v.size(), reason);
		v.clear();
	}
}

void TinsSniffer::onData(Tins::TCPStream &st) const
{
	for (auto &cb : m_callbacks)
	{
		if (cb) {
			auto id = convertID(st.stream_info());
			callCallback(cb, id, st.client_payload(), CallReasonRequest);
			callCallback(cb, id, st.server_payload(), CallReasonResponse);
		}
	}
}

void TinsSniffer::onClose(Tins::TCPStream &st) const
{
	for (auto &cb : m_callbacks)
	{
		if (cb) {
			cb(convertID(st.stream_info()), nullptr, 0, CallReasonClose);
		}
	}
}

const char *TinsSniffer::defaultFilter = "";
