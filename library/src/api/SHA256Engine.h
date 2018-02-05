//
// Created by Dawid Drozd aka Gelldur on 05.02.18.
//
#pragma once

#include <Poco/Crypto/DigestEngine.h>

namespace Api
{

class SHA256Engine : public Poco::Crypto::DigestEngine
{
	using inherited = Poco::Crypto::DigestEngine;

public:
	enum
	{
		BLOCK_SIZE = 64,
		DIGEST_SIZE = 32
	};

	SHA256Engine()
			: inherited("SHA256")
	{
	}
};
}


