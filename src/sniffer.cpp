#include "sniffer.hpp"
#include <cassert>


StreamID convertID(const Tins::TCPStream::StreamInfo &info)
{
	return StreamID(info.client_addr, info.client_port);
}

TinsSniffer::TinsSniffer(int interface, bool promiscMode)
{
	Tins::SnifferConfiguration config;
	config.set_buffer_size(1024 * 1024 * 50);
	config.set_promisc_mode(promiscMode);
	auto x=Tins::NetworkInterface::all();
	for (auto n : x){
		printf("%s\n", n.name().data());
		printf("%s\n",n.addresses().ip_addr.to_string().data());
	}
	m_tinsSniffer = std::make_unique<Tins::Sniffer>(Tins::NetworkInterface::all().at(interface).name(), config);
	m_tinsSniffer->set_filter(defaultFilter);
}

TinsSniffer::~TinsSniffer()
{
}

void TinsSniffer::startSniff()
{
	assert(m_tinsSniffer);
	Tins::TCPStreamFollower follower;
	auto dataFun = [this](Tins::TCPStream &st) {
		for (auto &cb : m_callbacks)
		{
			if (cb) {
				auto id = convertID(st.stream_info());
				promot(cb, id, st.client_payload(), CallReasonRequest);
				promot(cb, id, st.server_payload(), CallReasonResponse);
			}
		}
	};
	auto closeFun = [this](Tins::TCPStream &st) {
		for (auto &cb : m_callbacks)
		{
			if (cb) {
				cb(convertID(st.stream_info()), nullptr, 0, CallReasonClose);
			}
		}
	};
	try {
		std::lock_guard<std::mutex> l(m_mutex);
		follower.follow_streams(*m_tinsSniffer, dataFun, closeFun);
	}
	catch (std::exception e){
		printf("s");
	}
	catch (...) {
		printf("s");
	}
}

void TinsSniffer::stopSniff()
{
	pcap_breakloop(m_tinsSniffer->get_pcap_handle());
	std::lock_guard<std::mutex> l(m_mutex);
}

void TinsSniffer::setFilter(const std::string &filter)
{
	assert(m_tinsSniffer);
	m_tinsSniffer->set_filter(filter);
}


void TinsSniffer::promot(SnifferCallback &cb, const StreamID &id, std::vector<uint8_t> &v, CallReason reason)
{
	if (!v.empty()) {
		cb(id, (const char*)v.data(), v.size(), reason);
		v.clear();
	}
}

const char *TinsSniffer::defaultFilter = "tcp port 80";

Sniffer::Sniffer()
{
}

Sniffer::~Sniffer()
{
}

void Sniffer::addSnifferCallback(SnifferCallback consumer)
{
	m_callbacks.push_back(consumer);
}
