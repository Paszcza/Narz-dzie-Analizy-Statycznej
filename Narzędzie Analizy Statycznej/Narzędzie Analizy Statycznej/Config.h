#ifndef NAS_CONFIG
#define NAS_CONFIG

#include <string>
#include <set>

using ConfigTaintData = struct {

	std::set<std::string> sources;
	std::set<std::string> sinks;
	std::set<std::string> sanitizers;
};

ConfigTaintData getDataAnalysisConfig();

#endif
