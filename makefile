all:
	mpic++ -g Writer.cpp -o writer -ladios2 -std=c++11
	mpic++ -g Reader.cpp -o reader -ladios2 -std=c++11

