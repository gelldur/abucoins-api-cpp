CMAKE_MINIMUM_REQUIRED(VERSION 3.2 FATAL_ERROR)

#Path wehere we install lib (default path from README sample)
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_SOURCE_DIR}/../library/build/install/cmake/")

FIND_PACKAGE(Poco 1.8 REQUIRED Foundation Crypto Util JSON NetSSL)
FIND_PACKAGE(AbucoinsApi REQUIRED)

ADD_EXECUTABLE(Sample
		src/main.cpp
		)

SET_TARGET_PROPERTIES(Sample PROPERTIES
		CXX_STANDARD 14
		)

TARGET_LINK_LIBRARIES(Sample PUBLIC
		AbucoinsApi
		)
