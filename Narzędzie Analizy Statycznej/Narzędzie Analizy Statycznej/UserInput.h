#ifndef NAS_USER_INPUT
#define NAS_USER_INPUT

#include <iostream>
#include <vector>

using UserInput = struct {

	std::string outputFileName;
	std::string startingFunction;
	std::vector<std::string> inputFileNames;
};

UserInput parseUserInput(int argc, char** argv);

#endif
