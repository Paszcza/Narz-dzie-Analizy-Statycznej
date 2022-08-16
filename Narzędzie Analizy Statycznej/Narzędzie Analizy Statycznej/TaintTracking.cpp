#include "TaintTracking.h"

#include "ASTTraversal.h"
#include "Config.h"

#include <iostream>
#include <vector>

bool resolveMultipleNodesTaint(std::vector<TaintNode> taintNodeList, std::vector<int> nodes) {

	for (int node : nodes) {
		if (taintNodeList[node].tainted == true) {
			return true;
		}
	}
	return false;
}

bool trackThroughFunction(std::string functionName, FunctionMap functionMap, ConfigTaintData configTaintData, std::vector<Event>* eventList, std::vector<bool> paramsTaint, std::vector<Param> params) {

	bool returnTaint = false;
	std::vector<TaintNode> taintNodeList = {};
	const DataFlowGraph dfGraph = functionMap.at(functionName).dfGraph;
	for (GraphNodeData nodeData : dfGraph.nodeList) {
		taintNodeList.push_back({ false, {} });
	}
	for (int i = 0; i < paramsTaint.size(); i++) {
		taintNodeList[i].tainted = paramsTaint[i];
		if (params[i].isRef) {
			taintNodeList[i].refPath.insert(i);
		}
	}

	for (int nodeIndex = 0; nodeIndex < dfGraph.nodeList.size(); nodeIndex++) {

		GraphNodeData graphNodeData = dfGraph.nodeList[nodeIndex];
		if (dfGraph.functionCallMap.find(nodeIndex) != dfGraph.functionCallMap.end()) {

			FunctionCallNode callNode = dfGraph.functionCallMap.at(nodeIndex);
			std::vector<Param> referenceParams = {};
			for (Param param : callNode.params) {
				if (param.isRef) {
					referenceParams.push_back(param);
				}
			}
			std::vector<bool> paramsTaint;
			for (Param param : callNode.params) {
				paramsTaint.push_back(resolveMultipleNodesTaint(taintNodeList, param.nodes));
				if (paramsTaint.back()) {
					eventList->push_back({ EventType::taint_to_call, functionMap.at(functionName).file, functionName, graphNodeData.line, graphNodeData.column, nullptr });
				}
			}

			if (configTaintData.sources.find(callNode.name) != configTaintData.sources.end()) {

				int refParamCount = referenceParams.size();
				for (Param param : referenceParams)
					for(int node : param.nodes) {
						taintNodeList[node].tainted = true;
				}
				eventList->push_back({ EventType::source_detected, functionMap.at(functionName).file, functionName, graphNodeData.line, graphNodeData.column, &refParamCount});
				taintNodeList[nodeIndex].tainted = true;
			}
			else if (configTaintData.sanitizers.find(callNode.name) != configTaintData.sanitizers.end()) {

				for (Param param : callNode.params) {
					for (int node : param.nodes) {
						if (taintNodeList[node].tainted) {
							eventList->push_back({ EventType::taint_to_sanitizer, functionMap.at(functionName).file, functionName, graphNodeData.line, graphNodeData.column, nullptr });
							taintNodeList[node].tainted = false;
						}
					}
				}
				taintNodeList[nodeIndex].tainted = false;
			}
			else if (configTaintData.sinks.find(callNode.name) != configTaintData.sinks.end()) {

				for (Param param : callNode.params) {
					for (int node : param.nodes) {
						if (taintNodeList[node].tainted) {
							eventList->push_back({ EventType::taint_to_sink, functionMap.at(functionName).file, functionName, graphNodeData.line, graphNodeData.column, nullptr });
						}
					}
				}
				if (functionMap.find(callNode.name) != functionMap.end()) {
					taintNodeList[nodeIndex].tainted = trackThroughFunction(callNode.name, functionMap, configTaintData, eventList, paramsTaint, callNode.params);
				}
				else {
					for(bool paramTaint : paramsTaint) {
						if (paramTaint) {
							taintNodeList[nodeIndex].tainted;
						}
					}
				}
			}
			else {
				if (functionMap.find(callNode.name) != functionMap.end()) {
					taintNodeList[nodeIndex].tainted = trackThroughFunction(callNode.name, functionMap, configTaintData, eventList, paramsTaint, callNode.params);
				}
				else {
					for (bool paramTaint : paramsTaint) {
						if (paramTaint) {
							taintNodeList[nodeIndex].tainted;
						}
					}
				}
			}
		}
		if (taintNodeList[nodeIndex].tainted) {

			if (!taintNodeList[nodeIndex].refPath.empty()) {
				std::set<int> paramIndexSet = taintNodeList[nodeIndex].refPath;
				eventList->push_back({ EventType::taint_to_reference, functionMap.at(functionName).file, functionName, graphNodeData.line, graphNodeData.column, &paramIndexSet });
			}
			for (int adjacentNode : graphNodeData.adjacency) {

				taintNodeList[adjacentNode].tainted = true;
				for (int refParam : taintNodeList[nodeIndex].refPath) {
					taintNodeList[adjacentNode].refPath.insert(refParam);
				}
			}
		}
	}
	for (int returnNode : dfGraph.returnNodes) {
		if (taintNodeList[returnNode].tainted) {
			returnTaint = true;
		};
	}
	return returnTaint;
}
