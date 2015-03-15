#include <string>
#include <iostream>
#include "storage.hpp"
#include "stream.hpp"
#include "sniffer_stream.hpp"

using namespace std;

void main()
{
	auto storage = PersistentStorageFactory::build("1024");
	storage->loadAll();

	SnifferStream stream(3);
	stream.addStreamCallback([&storage](const StreamID &id, shared_ptr<CachePacket> pkt) {
		if (pkt->request && pkt->response) {
			cout << pkt->url << endl;
			storage->add(pkt->url, pkt);
		}
	});
	stream.startSniff();
}
