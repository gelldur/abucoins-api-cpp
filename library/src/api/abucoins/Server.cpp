//
// Created by Dawid Drozd aka Gelldur on 05.02.18.
//
#include <api/abucoins/Server.h>
//////////////////////////////////

#include <chrono>

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/HMACEngine.h>

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

#include <Poco/LogStream.h>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <api/abucoins/Order.h>

#include <api/SHA256Engine.h>

namespace
{
std::chrono::seconds getUnixTimeStamp()
{
	const auto now = std::chrono::system_clock::now();
	const auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::seconds>(duration);
}
}

namespace Api
{
namespace Abucoins
{

Server::Configuration::Configuration(const std::string& secret
									 , const std::string& passPhrase
									 , const std::string& key)
		: secret(secret)
		, passPhrase(passPhrase)
		, key(key)
{
}

Server::Server(Server::Configuration configuration)
		: _configuration(configuration)
{
	_session = std::unique_ptr<Poco::Net::HTTPClientSession>(
			new Poco::Net::HTTPSClientSession("api.abucoins.com"));
	_session->setKeepAlive(true);
	_session->setTimeout(Poco::Timespan{20, 0});
}

Server::~Server() = default;

void Server::setConfiguration(const Server::Configuration& configuration)
{
	_configuration = configuration;
}

void Server::setDebug(bool isDebug)
{
	_debug = isDebug;
}

void Server::createOrder(Order& order)
{
	//Validate
	if (order.productId.empty())
	{
		throw std::invalid_argument{"Missing product id"};
	}
	if (order.size.empty())
	{
		throw std::invalid_argument{"Missing size"};
	}
	if (order.price.empty())
	{
		throw std::invalid_argument{"Missing price"};
	}

	Poco::JSON::Object orderJson;
	orderJson.set("product_id", order.productId);
	orderJson.set("size", order.size);
	orderJson.set("price", order.price);

	if (order.side == Order::Type::buy)
	{
		orderJson.set("side", "buy");
	}
	else if (order.side == Order::Type::sell)
	{
		orderJson.set("side", "sell");
	}
	else
	{
		throw std::invalid_argument{"Unknown type for 'side'"};
	}

	orderJson.set("type", "limit");
	//GTT - Good Till Time
	//GTC - Good Till Cancel
	//IOC - Immediate Or Cancel
	//FOK - Fill Or Kill
	orderJson.set("time_in_force", "GTC");
	orderJson.set("stp", "co");

	const auto root = post("/orders", orderJson);
	const auto& rootObject = root.extract<Poco::JSON::Object::Ptr>();

	parseOrder(order, *rootObject);
}

bool Server::removeOrder(const Order& order)
{
	if (order.id.empty())
	{
		throw std::invalid_argument{"Missing order id"};
	}

	std::stringstream url;
	url << "/orders/" << order.id;

	const auto root = httpDelete(url.str());
	const auto& rootArray = root.extract<Poco::JSON::Array::Ptr>();

	if (rootArray->size() < 1)
	{
		throw std::invalid_argument{"Order is missing"};
	}

	auto id = rootArray->get(0).convert<std::string>();
	return order.id == id;
}

unsigned Server::removeAllOrders(const std::string& productId)
{
	std::stringstream url;
	url << "/orders";
	if (productId.empty() == false)
	{
		url << "?product_id=" << productId;
	}

	const auto root = httpDelete(url.str());
	const auto& rootArray = root.extract<Poco::JSON::Array::Ptr>();
	return rootArray->size();
}

void Server::updateOrder(Order& order)
{
	if (order.id.empty())
	{
		throw std::invalid_argument{"Missing order id"};
	}

	std::stringstream url;
	url << "/orders/" << order.id;
	const auto root = get(url.str(), true);
	const auto& rootObject = root.extract<Poco::JSON::Object::Ptr>();

	parseOrder(order, *rootObject);
}

void Server::prepareRequest(Poco::Net::HTTPRequest& request, const std::string& body)
{
	const auto timeStamp = std::to_string(getUnixTimeStamp().count());

	std::string secretDecoded;
	std::istringstream secretStream(_configuration.secret);
	Poco::Base64Decoder decoder{secretStream}; // remember to close
	decoder >> secretDecoded;
	if (_debug)
	{
		Poco::LogStream{Poco::Logger::get("Abucoins")}.debug()
				<< "Signing request\n###########################################"
				<< "SecretDecoded: " << secretDecoded << std::endl;
	}

	Poco::HMACEngine<SHA256Engine> hmac{secretDecoded};
	std::stringstream preHashStream;
	preHashStream << timeStamp << request.getMethod() << request.getURI() << body;
	if (_debug)
	{
		Poco::LogStream{Poco::Logger::get("Abucoins")}.debug()
				<< "String: " << preHashStream.str()
				<< "\n###########################################" << std::endl;
	}
	hmac.update(preHashStream.str());

	std::stringstream signStream;
	Poco::Base64Encoder encoder{signStream};
	const auto& bytes = hmac.digest();
	encoder.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());

	encoder.close(); // remember about closing!

	request.set("AC-ACCESS-KEY", _configuration.key);
	request.set("AC-ACCESS-PASSPHRASE", _configuration.passPhrase);
	request.set("AC-ACCESS-SIGN", signStream.str());
	request.set("AC-ACCESS-TIMESTAMP", timeStamp);
}

Poco::Dynamic::Var Server::get(const std::string method, bool sign)
{
	Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_GET
			, method
			, Poco::Net::HTTPRequest::HTTP_1_1};
	request.setKeepAlive(true);

	if (sign)
	{
		prepareRequest(request);
	}

	if (_debug)
	{
		Poco::LogStream stream{Poco::Logger::get("Abucoins")};
		stream.debug();
		stream << "------------------------------------------------\n";
		stream << "↪ REQUEST ↪\n";
		stream << _session->getHost() << "\n";
		request.write(stream);
		stream << "------------------------------------------------\n";
		stream << std::endl;
	}

	_session->sendRequest(request);

	Poco::Net::HTTPResponse response;
	const std::string responseBodyString = [&]()
	{//Lambda for nice and clean stack
		std::istream& responseBody = _session->receiveResponse(response);
		return std::string(std::istreambuf_iterator<char>(responseBody), {});
	}();

	if (_debug)
	{
		Poco::LogStream stream{Poco::Logger::get("Abucoins")};
		stream.debug();
		stream << "------------------------------------------------\n";
		stream << "↩ RESPONSE ↩\n";
		response.write(stream);
		stream << "Data:\n" << responseBodyString << "\n";
		stream << std::endl;
	}

	Poco::JSON::Parser parser;
	return parser.parse(responseBodyString);
}

Poco::Dynamic::Var Server::httpDelete(const std::string method)
{
	Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_DELETE
			, method
			, Poco::Net::HTTPRequest::HTTP_1_1};
	request.setKeepAlive(true);

	prepareRequest(request);

	if (_debug)
	{
		Poco::LogStream stream{Poco::Logger::get("Abucoins")};
		stream.debug();
		stream << "------------------------------------------------\n";
		stream << "↪ REQUEST ↪ ";
		stream << _session->getHost() << "\n";
		request.write(stream);
		stream << "------------------------------------------------\n";
		stream << std::endl;
	}

	_session->sendRequest(request);

	Poco::Net::HTTPResponse response;
	const std::string responseBodyString = [&]()
	{//Lambda for nice and clean stack
		std::istream& responseBody = _session->receiveResponse(response);
		return std::string(std::istreambuf_iterator<char>(responseBody), {});
	}();

	if (_debug)
	{
		Poco::LogStream stream{Poco::Logger::get("Abucoins")};
		stream.debug();
		stream << "------------------------------------------------\n";
		stream << "↩ RESPONSE ↩\n";
		response.write(stream);
		stream << "Data:\n" << responseBodyString << "\n";
		stream << std::endl;
	}

	Poco::JSON::Parser parser;
	return parser.parse(responseBodyString);
}

Poco::Dynamic::Var Server::post(const std::string& method, const Poco::JSON::Object& body)
{
	Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_POST
			, method
			, Poco::Net::HTTPRequest::HTTP_1_1};
	request.setKeepAlive(true);
	request.setContentType("application/json");

	const std::string bodyRawData = [&]()
	{//Lambda for nice and clean stack
		std::stringstream bodyStream;
		body.stringify(bodyStream);
		return bodyStream.str();
	}();

	request.setContentLength(bodyRawData.size());
	prepareRequest(request, bodyRawData);

	if (_debug)
	{
		Poco::LogStream stream{Poco::Logger::get("Abucoins")};
		stream.debug();
		stream << "------------------------------------------------\n";
		stream << "↪ REQUEST ↪ ";
		stream << _session->getHost() << "\n";
		request.write(stream);
		stream << "------------------------------------------------\n";

		stream << "--------------------- BODY ---------------------\n";
		stream << "------------------------------------------------\n";
		body.stringify(stream, 2);
		stream << "\n------------------------------------------------\n";
		stream << std::endl;
	}

	{
		std::ostream& sink = _session->sendRequest(request);
		sink << bodyRawData;//Send data to sink
	}

	Poco::Net::HTTPResponse response;
	const std::string responseBodyString = [&]()
	{//Lambda for nice and clean stack
		std::istream& responseBody = _session->receiveResponse(response);
		return std::string(std::istreambuf_iterator<char>(responseBody), {});
	}();

	if (_debug)
	{
		Poco::LogStream stream{Poco::Logger::get("Abucoins")};
		stream.debug();
		stream << "------------------------------------------------\n";
		stream << "↩ RESPONSE ↩\n";
		response.write(stream);
		stream << "Data:\n" << responseBodyString << "\n";
		stream << std::endl;
	}

	Poco::JSON::Parser parser;
	return parser.parse(responseBodyString);
}

void Server::parseOrder(Order& order, const Poco::JSON::Object& object)
{
	order.id = object.getValue<std::string>("id");
	order.price = object.getValue<std::string>("price");
	order.size = object.getValue<std::string>("size");
	order.status = object.getValue<std::string>("status");
	order.filledSize = object.getValue<std::string>("filled_size");

	const auto side = object.getValue<std::string>("side");
	if (side == "buy")
	{
		order.side = Order::Type::buy;
	}
	else if (side == "sell")
	{
		order.side = Order::Type::sell;
	}
	else
	{
		order.side = Order::Type::unknown;
		throw std::invalid_argument("Unknown: " + side);
	}
}

}
}
