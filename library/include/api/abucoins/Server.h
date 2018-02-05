//
// Created by Dawid Drozd aka Gelldur on 05.02.18.
//
#pragma once

#include <string>
#include <memory>

namespace Poco
{
namespace JSON
{
class Object;
}
namespace Dynamic
{
class Var;
}

namespace Net
{
class HTTPClientSession;
class HTTPRequest;
}
}

namespace Api
{
namespace Abucoins
{

struct Order;

class Server
{
public:
	struct Configuration
	{
		std::string secret;
		std::string passPhrase;
		std::string key;

		Configuration(const std::string& secret
					  , const std::string& passPhrase
					  , const std::string& key);
		Configuration() = default;
	};

	Server(Configuration configuration);
	Server() = default;
	~Server();

	///Print to logger extra logs
	void setDebug(bool isDebug);

	void setConfiguration(const Configuration& configuration);

	/// Create new order http://docs.abucoins.com/#create-an-order
	void createOrder(Order& order);

	/// Remove single order http://docs.abucoins.com/#delete-an-single-order
	bool removeOrder(const Order& order);

	/**
	 * Remove all orders https://docs.abucoins.com/#delete-all-orders
	 * @param productId optional product id
	 * @return count of removed orders
	 */
	unsigned removeAllOrders(const std::string& productId = "");

	/// Get order http://docs.abucoins.com/#get-an-order
	void updateOrder(Order& order);

private:
	Configuration _configuration = {};

	std::unique_ptr<Poco::Net::HTTPClientSession> _session;

	bool _debug = false;

	void prepareRequest(Poco::Net::HTTPRequest& request, const std::string& body = "");
	Poco::Dynamic::Var get(const std::string method, bool sign);
	Poco::Dynamic::Var httpDelete(const std::string method);
	Poco::Dynamic::Var post(const std::string& method, const Poco::JSON::Object& body);

	void parseOrder(Order& order, const Poco::JSON::Object& object);
};

}
}
