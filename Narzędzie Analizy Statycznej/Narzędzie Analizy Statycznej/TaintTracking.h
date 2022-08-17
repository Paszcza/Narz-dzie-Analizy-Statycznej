#ifndef NAS_TAINT_TRACKING
#define NAS_TAINT_TRACKING

#include "ASTTraversal.h"
#include "Config.h"

#include <set>
#include <vector>

enum class EventType {

	source_detected = 0,
	taint_to_sink = 1,
	taint_to_sanitizer = 2,
	function_call = 3,
	taint_to_reference = 4,
	taint_to_call = 5
};

using Event = struct {

	EventType type;
	std::string file;
	std::string function;
	int line;
	int column;
	void* extraData;
};

using TaintNode = struct {

	bool tainted;
	std::set<int> refPath;
};

bool trackThroughFunction(
	std::string functionName, FunctionMap functionMap,
	ConfigTaintData configTaintData, std::vector<Event>* eventList,
	std::vector<bool>* paramsTaint, std::vector<Param> params
);

#endif
