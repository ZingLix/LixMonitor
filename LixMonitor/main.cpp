#include "Server.h"
#include "InfoGenerator.h"
#include <iostream>

int main(int argc, char* argv[]) {
	Server s(9981);
	s.start();
}

