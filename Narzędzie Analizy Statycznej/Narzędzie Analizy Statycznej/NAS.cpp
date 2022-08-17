#include "ASTTraversal.h"
#include "Config.h"
#include "LibclangHandling.h"
#include "ReportPrinter.h"
#include "TaintTracking.h"
#include "UserInput.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {

    UserInput userInput = parseUserInput(argc, argv);
    ConfigTaintData configTaintData = getDataAnalysisConfig();

    FunctionMap functionMap = {};
    std::vector<Event> eventList;
    if (!userInput.inputFileNames.empty()) {

        for (std::string fileName : userInput.inputFileNames) {
            traverseFile(fileName, &functionMap);
        }
        if (functionMap.find(userInput.startingFunction) != functionMap.end()) {
            
            std::vector<bool> paramTaint = {};
            if (userInput.startingFunction == "main") {
                paramTaint = {true, true};
            }
            trackThroughFunction(userInput.startingFunction, functionMap, configTaintData, &eventList, &paramTaint, {});
        }
        else {
            std::cout << "ERROR: Couldn't find the starting function in the files." << std::endl;
        }
        printReport(eventList, userInput.outputFileName);
    }
    else {
        std::cout << "ERROR: No files specified." << std::endl;
    }
    return 0;
}