#include <memory>
#include <string>
#include "../src/server.hpp"
using namespace std;

void main()
{
	auto server = ServerFactory::build(1024);
	server->setListener([](const string &url) -> shared_ptr<Response> {
		return shared_ptr<Response>(ResponseBuilder::build(url + "Google Test",{{"Content-Length",to_string(11 + url.size())}}, 200, "OK"));
	});
	server->start();
}
