#include "UserInput.h"

#include <fstream>
#include <iostream>
#include <vector>

UserInput parseUserInput(int argc, char** argv)
{

    std::vector<std::string> inputFileNames;
    std::string outputFileName = std::string();
    std::string startingFunction = "main";

    for (int i = 1; i < argc; i++) {

        if (argv[i][0] == '-') {
            switch (argv[i][1]) {

            case 'o':
                if (i + 1 >= argc) {
                    std::cout << "ERROR: no output filename given. The report will be printed in the command line." << std::endl;
                }
                else {
                    outputFileName = argv[++i];
                }
                break;
            case 'f':
                if (i + 1 >= argc) {
                    std::cout << "ERROR: no function name given. The default starting function is \"main\"." << std::endl;
                }
                else {
                    startingFunction = argv[++i];
                }
                break;
            }
        }
        else {
            inputFileNames.push_back(argv[i]);
        }
    }
    UserInput userInput = { outputFileName, startingFunction, inputFileNames };
    return userInput;
}
