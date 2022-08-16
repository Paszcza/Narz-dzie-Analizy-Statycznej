#include "Config.h"

#include <fstream>
#include <set>
#include <string>

#include <iostream>

extern const std::set<std::string> g_sources;

ConfigTaintData getDataAnalysisConfig()
{
    ConfigTaintData newConfigTaintData;

    std::fstream fileSources;
    std::fstream fileSinks;
    std::fstream fileSanitizers;
    std::string str;

    fileSources.open("sources.config");
    while (std::getline(fileSources, str)) {
        newConfigTaintData.sources.insert(str);
    }
    fileSources.close();

    fileSinks.open("sinks.config");
    while (std::getline(fileSinks, str)) {
        newConfigTaintData.sinks.insert(str);
    }
    fileSinks.close();

    fileSanitizers.open("sanitizers.config");
    while (std::getline(fileSanitizers, str)) {
        newConfigTaintData.sanitizers.insert(str);
    }
    fileSanitizers.close();

    return newConfigTaintData;
}