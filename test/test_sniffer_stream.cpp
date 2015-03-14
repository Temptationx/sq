#include <thread>
#include <iostream>
#include <string>
#include "../src/sniffer_stream.hpp"

using namespace std;


void main()
{
	SnifferStream w(4);
	w.startSniff();
}

