#ifndef NAS_AST_TRAVERSAL
#define NAS_AST_TRAVERSAL

#include "LibclangHandling.h"

#include "clang-c/Index.h"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using Param = struct {
	
	std::vector<int> nodes;
	bool isRef;
};

using FunctionCallNode = struct {

	std::string usr;
	std::string name;
	std::vector<Param> params;
};

using GraphNodeData = struct {

	std::vector<int> adjacency;
	int line;
	int column;
};

using DataFlowGraph = struct {

	int paramCount;
	std::vector<GraphNodeData> nodeList;
	std::map<int, FunctionCallNode> functionCallMap;
	std::vector<int> returnNodes;
};

using FunctionData = struct {
	
	std::string file;
	DataFlowGraph dfGraph;
};

using FunctionMap = std::map<std::string, FunctionData>;

using VarDictionary = std::map<std::string, int>;

void traverseFile(std::string fileName, FunctionMap* functionMap);

void debugPrintGraphs(FunctionMap fMap);

#endif
