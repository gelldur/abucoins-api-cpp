//
// Created by Dawid Drozd aka Gelldur on 05.02.18.
//

#include <thread>
#include <chrono>
#include <iostream>

#include <Poco/ConsoleChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>

#include <Poco/LogStream.h>
#include <Poco/AutoPtr.h>

#include <api/abucoins/Server.h>
#include <api/abucoins/Order.h>

int main()
{
	// global setup...
	Poco::AutoPtr<Poco::Channel> channelConsole;
	{
		Poco::AutoPtr<Poco::ConsoleChannel> console{new Poco::ConsoleChannel{}};

		//{%I} not working now (thread id)
		//FIXME Time is in UTC ?
		Poco::AutoPtr<Poco::PatternFormatter> patternFormatter(new Poco::PatternFormatter{
				"%H:%M:%S [%p] (%s) %t"});
		Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
				new Poco::FormattingChannel{patternFormatter, console});

		channelConsole = formattingChannel;
	}

	Poco::Logger::root().setChannel(channelConsole);
	Poco::Logger::root().setLevel(Poco::Message::PRIO_TRACE);

	using namespace std::chrono;

	Poco::LogStream stream{Poco::Logger::get("SampleApp")};

	Api::Abucoins::Server::Configuration configuration;
	configuration.passPhrase = "ABC**123***";//Paste it here from website
	configuration.key = "3210513824-MK******6G4******39ZE5U*****PW1";
	configuration.secret = "PDZC********SkhSdzopMGFBLV*******FMufVpe******lMWo3*****Rixr";

	Api::Abucoins::Server server{configuration};
	server.setDebug(true);

	server.removeAllOrders("ZEC-BTC");

	Api::Abucoins::Order order;
	order.side = Api::Abucoins::Order::Type::buy;
	order.size = "1";
	order.price = "0.00000004";
	order.productId = "ZEC-BTC";

	server.createOrder(order);
	stream << "Order: " << order.status << " " << order.filledSize << std::endl;
	stream << "[server.createOrder] Wait for 5s" << std::endl;
	std::this_thread::sleep_for(5s);

	server.updateOrder(order);
	stream << "Order: " << order.status << " " << order.filledSize << std::endl;
	stream << "[server.updateOrder] Wait for 5s" << std::endl;
	std::this_thread::sleep_for(5s);

	server.removeOrder(order);
	stream << "[server.removeOrder] Wait for 5s" << std::endl;
	std::this_thread::sleep_for(5s);

	server.updateOrder(order);
	stream << "[server.updateOrder] Wait for 5s" << std::endl;
	stream << "Order: " << order.status << " " << order.filledSize << std::endl;

	return 0;
}
