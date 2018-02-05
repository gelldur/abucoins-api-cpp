CMAKE_MINIMUM_REQUIRED(VERSION 3.2 FATAL_ERROR)

PROJECT(AbucoinsApi
		VERSION 0.9.0 # Not related to api.abucoins version
		LANGUAGES CXX
		)

# BUILD_SHARED_LIBS can controll build type!
ADD_LIBRARY(AbucoinsApi
		src/api/abucoins/Server.cpp include/api/abucoins/Server.h
		src/api/abucoins/Order.cpp include/api/abucoins/Order.h
		src/api/SHA256Engine.h
		)

TARGET_INCLUDE_DIRECTORIES(AbucoinsApi PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
		$<INSTALL_INTERFACE:include/>
		PRIVATE src/
		)

TARGET_COMPILE_OPTIONS(AbucoinsApi PRIVATE
		-Wall -pedantic
		-Wnon-virtual-dtor
		-Werror
		-Wno-error=deprecated-declarations
		)

SET_TARGET_PROPERTIES(AbucoinsApi PROPERTIES
		CXX_STANDARD 14
		)
TARGET_LINK_LIBRARIES(AbucoinsApi PRIVATE
		Poco::Foundation
		Poco::Crypto
		Poco::Util
		Poco::JSON
		Poco::NetSSL
		)

FIND_PACKAGE(Poco 1.8 REQUIRED Foundation Crypto Util JSON NetSSL)

INSTALL(TARGETS AbucoinsApi EXPORT AbucoinsApiConfig
		ARCHIVE DESTINATION lib/
		LIBRARY DESTINATION lib/
		RUNTIME DESTINATION bin/
		INCLUDES DESTINATION include/
		)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/ FILES_MATCHING PATTERN "*.h*")
INSTALL(EXPORT AbucoinsApiConfig
		DESTINATION cmake/
		)
EXPORT(TARGETS AbucoinsApi FILE AbucoinsApi.cmake)

