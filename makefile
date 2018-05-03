all:
	mpic++  helloSstWriter.cpp -o writer -ladios2 -std=c++11
	mpic++  helloSstReader.cpp -o reader -ladios2 -std=c++11

