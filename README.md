# abucoins-api-cpp

Sample implementation in C++

## Requirements

- cmake
- [Poco library](https://pocoproject.org/) 
- clang / gcc

## Sample

In [main.cpp](sample/src/main.cpp) you need to update those lines
```cpp
Api::Abucoins::Server::Configuration configuration;
configuration.passPhrase = "ABC**123***";//Paste it here from website
configuration.key = "3210513824-MK******6G4******39ZE5U*****PW1";
configuration.secret = "PDZC********SkhSdzopMGFBLV*******FMufVpe******lMWo3*****Rixr";
```

Just pass here your api config. **Remember to grant rights to your api key.**

## How to play with on Linux

```
#We should be in project root
cd library/
mkdir -p build/
cd build/

#Generate project for AbucoinsApi library
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=./install ..
make
make install

cd ../.. #We should be in project root

cd sample/
mkdir -p build/
cd build/

#Generate project for SampleApp
cmake -DCMAKE_BUILD_TYPE=Debug ..
#Build app
make
#Run sample app
./Sample
```

## Other way

- You can add library as subdirectory to your project.
- You can copy sample code directly to your project and improve it 😄

## Questions & improvements

Please create issues if you want to ask something or suggest improvement.
