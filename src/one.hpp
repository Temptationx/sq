#include <thread>
#include <memory>
#include "type.hpp"
#include "storage.hpp"
#include "sniffer_stream.hpp"
#include "server.hpp"
#include "proxy.hpp"
using namespace std;

namespace Poco{
	namespace Net{
		class HTTPServer;
	}
}

class Sq
{
public:
	Sq(const std::string dir, int inter, int proxy_server_port);
	~Sq();
	void start();
	void sync();
	void stop();
	void enable_log(const std::string &log_filename);
	PersistentStorage* storage();
private:
	void link();
	std::unique_ptr<SnifferStream> stream;
	std::unique_ptr<PersistentStorage> storage_;
	std::unique_ptr<Proxy> proxy_;
	std::unique_ptr<IServer> server;
	std::thread mm;
	std::thread ss;
	int stoped = 0;
	Poco::Net::HTTPServer *control_server = nullptr;
};
