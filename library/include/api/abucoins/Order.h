//
// Created by Dawid Drozd aka Gelldur on 05.02.18.
//
#pragma once

#include <string>

namespace Api
{
namespace Abucoins
{

struct Order
{
	enum class Type
	{
		unknown = 0, buy, sell
	};

	std::string id;
	std::string productId;
	std::string size;
	std::string price;
	std::string status = "unknown";
	std::string filledSize;
	Type side = Type::unknown;
};

}
}

