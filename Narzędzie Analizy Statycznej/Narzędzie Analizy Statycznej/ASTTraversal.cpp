#include "ASTTraversal.h"

#include "LibclangHandling.h"

#include "clang-c/Index.h"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

DataFlowGraph traverseFunction(CXCursor cxCursor, std::vector<std::string> globalVars);
void traverseCompoundStatement(DataFlowGraph* dfGraph, VarDictionary* varDict, CXCursor cxRootCursor);
std::vector<std::string> traverseExpression(DataFlowGraph* dfGraph, VarDictionary* varDict, CXCursor cxRootCursor, int rootNode);

int createNode(DataFlowGraph* dfGraph, CXCursor cxCursor) {

	unsigned line, column;
	clang_getExpansionLocation(clang_getCursorLocation(cxCursor), nullptr, &line, &column, nullptr);
	dfGraph->nodeList.push_back({ { }, (int)line, (int)column });
	return int(dfGraph->nodeList.size()) - 1;
}

int createNodeForNewVar(DataFlowGraph* dfGraph, VarDictionary* varDict, CXCursor cxCursor) {

	std::string varUSR = nas_getCursorUSR(cxCursor);
	if (varDict->find(varUSR) == varDict->end()) {

		int node = createNode(dfGraph, cxCursor);
		varDict->insert({ varUSR, node });
		return node;
	}
	return -1;
}

void connectNode(
	DataFlowGraph* dfGraph, VarDictionary varDict, int node,
	std::vector<std::string> graphSources,
	std::vector<std::string> graphDests) {

	for (int i = 0; i < graphSources.size(); i++) {
		dfGraph->nodeList[varDict.at(graphSources[i])].adjacency.push_back(node);
	}
	for (int i = 0; i < graphDests.size(); i++) {
		dfGraph->nodeList[node].adjacency.push_back(varDict.at(graphDests[i]));
	}
}

int createNodeFunctionCall(DataFlowGraph* dfGraph, VarDictionary* varDict, CXCursor cxRootCursor) {

	int newNode = createNode(dfGraph, cxRootCursor);
	FunctionCallNode newFunctionNode = { nas_getCursorUSR(cxRootCursor), nas_getCursorSpelling(cxRootCursor), {} };
	for (int i = 1; i < nas_getCursorChildCount(cxRootCursor); i++) {

		CXCursor cxChildCursor = nas_getCursorChild(cxRootCursor, i);
		newFunctionNode.params.push_back({ {}, false });
		if (clang_isExpression(clang_getCursorKind(cxChildCursor))) {

			std::vector<std::string> traversalResult = traverseExpression(dfGraph, varDict, cxChildCursor, newNode);
			for (auto& element : traversalResult) {

				if (varDict->find(element) != varDict->end()){
					newFunctionNode.params[i - 1].nodes.push_back(varDict->at(element));
				}
				if (nas_getCursorTypeKind(cxChildCursor) == "Pointer" || nas_getCursorTypeKind(cxChildCursor) == "IncompleteArray") {
					newFunctionNode.params[i - 1].isRef = true;
				}
			}
		}
	}
	dfGraph->functionCallMap.insert_or_assign(newNode, newFunctionNode);
	return newNode;
}

int createNodeAssignmentOp(DataFlowGraph* dfGraph, VarDictionary* varDict, CXCursor cxRootCursor, bool isCompound) {

	int node = createNode(dfGraph, cxRootCursor);
	std::vector<std::string> leftVars, rightVars;
	if (nas_getCursorChildCount(cxRootCursor) == 2) {

		leftVars = traverseExpression(dfGraph, varDict, nas_getCursorChild(cxRootCursor, 0), node);
		rightVars = traverseExpression(dfGraph, varDict, nas_getCursorChild(cxRootCursor, 1), node);
	}
	if (isCompound) {
		rightVars.insert(rightVars.begin(), leftVars.begin(), leftVars.end());
	}
	connectNode(dfGraph, *varDict, node, rightVars, {});
	varDict->insert_or_assign(leftVars[0], node);
	return node;
}

std::vector<std::string> traverseExpression(DataFlowGraph* dfGraph, VarDictionary* varDict, CXCursor cxRootCursor, int rootNode) {

	std::vector<std::string> exprVarList;
	switch (clang_getCursorKind(cxRootCursor)) {

	case CXCursorKind::CXCursor_DeclRefExpr:
		exprVarList.push_back(nas_getCursorUSR(cxRootCursor));
		break;
	case CXCursorKind::CXCursor_BinaryOperator:
		if (nas_cursorIsAssignmentOperator(cxRootCursor)) {

			int newOperationNode = createNodeAssignmentOp(dfGraph, varDict, cxRootCursor, false);
			if(rootNode >= 0 && newOperationNode != 0) dfGraph->nodeList[newOperationNode].adjacency.push_back(rootNode);
		}
		else {

			for (int i = 0; i < nas_getCursorChildCount(cxRootCursor); i++) {

				CXCursor cxChildCursor = nas_getCursorChild(cxRootCursor, i);
				std::vector<std::string> traversalResult = traverseExpression(dfGraph, varDict, cxChildCursor, rootNode);
				exprVarList.insert(exprVarList.end(), traversalResult.begin(), traversalResult.end());
			}
		}
		break;
	case CXCursorKind::CXCursor_CompoundAssignOperator:
	{
		int newOperationNode = createNodeAssignmentOp(dfGraph, varDict, cxRootCursor, true);
		if (rootNode >= 0) dfGraph->nodeList[newOperationNode].adjacency.push_back(rootNode);
	}
		break;
	case CXCursorKind::CXCursor_CallExpr:
	{
		int newFunctionNode = createNodeFunctionCall(dfGraph, varDict, cxRootCursor);
		if (rootNode >= 0) dfGraph->nodeList[newFunctionNode].adjacency.push_back(rootNode);
	}
		break;
	default:
		if (clang_isExpression(clang_getCursorKind(cxRootCursor))) {

			for (int i = 0; i < nas_getCursorChildCount(cxRootCursor); i++) {

				CXCursor cxChildCursor = nas_getCursorChild(cxRootCursor, i);
				std::vector<std::string> traversalResult = traverseExpression(dfGraph, varDict, cxChildCursor, rootNode);
				exprVarList.insert(exprVarList.end(), traversalResult.begin(), traversalResult.end());
			}
		}
		break;
	}
	return exprVarList;
}

void traverseCompoundStatement(DataFlowGraph* dfGraph, VarDictionary* varDict, CXCursor cxRootCursor) {

	for (int i = 0; i < nas_getCursorChildCount(cxRootCursor); i++) {

		CXCursor cxChildCursor = nas_getCursorChild(cxRootCursor, i);
		switch (clang_getCursorKind(cxChildCursor)) {

		case CXCursorKind::CXCursor_CompoundStmt:
		case CXCursorKind::CXCursor_IfStmt:
		case CXCursorKind::CXCursor_WhileStmt:
		case CXCursorKind::CXCursor_DoStmt:
		case CXCursorKind::CXCursor_ForStmt:
		case CXCursorKind::CXCursor_SwitchStmt:
		case CXCursorKind::CXCursor_CaseStmt:
			traverseCompoundStatement(dfGraph, varDict, cxChildCursor);
			break;
		case CXCursorKind::CXCursor_DeclStmt:
		{
			for (int i = 0; i < nas_getCursorChildCount(cxChildCursor); i++) {

				CXCursor cxDecl = nas_getCursorChild(cxChildCursor, i);
				if (clang_getCursorKind(cxDecl) == CXCursorKind::CXCursor_VarDecl) {

					int newNode = createNodeForNewVar(dfGraph, varDict, cxDecl);
					if (nas_getCursorChildCount(cxDecl) > 0) {

						CXCursor cxDeclChild = nas_getCursorChild(cxDecl, 0);
						if (clang_isExpression(clang_getCursorKind(cxDeclChild))) {
							connectNode(dfGraph, *varDict, newNode, traverseExpression(dfGraph, varDict, cxDeclChild, newNode), {});
						}
					}
				}
			}
		}
		break;
		case CXCursorKind::CXCursor_ReturnStmt:
		{
			int returnNode = createNode(dfGraph, cxChildCursor);
			connectNode(dfGraph, *varDict, returnNode, traverseExpression(dfGraph, varDict, nas_getCursorChild(cxChildCursor, 0), returnNode), {});
			dfGraph->returnNodes.push_back(returnNode);
		}
			break;
		default:
			if (clang_isExpression(clang_getCursorKind(cxChildCursor))) {
				traverseExpression(dfGraph, varDict, cxChildCursor, -1);
			}
		}
	}
}

DataFlowGraph traverseFunction(CXCursor cxRootCursor, std::vector<std::string> globalVars) {

	DataFlowGraph* dfGraph = new DataFlowGraph;
	VarDictionary* varDict = new VarDictionary;

	for (int i = 0; i < nas_getCursorChildCount(cxRootCursor); i++) {

		CXCursor cxChildCursor = nas_getCursorChild(cxRootCursor, i);
		switch (clang_getCursorKind(cxChildCursor)) {

		case CXCursorKind::CXCursor_ParmDecl:
			createNodeForNewVar(dfGraph, varDict, cxChildCursor);
			dfGraph->paramCount++;
			break;
		case CXCursorKind::CXCursor_CompoundStmt:
			traverseCompoundStatement(dfGraph, varDict, cxChildCursor);
			break;
		default:
			break;
		}
	}
	return *dfGraph;
}

void traverseFile(std::string fileName, FunctionMap* functionMap) {

	CXIndex cxIndex = clang_createIndex(1, 0);
	CXTranslationUnit cxTU = clang_parseTranslationUnit(cxIndex, fileName.c_str(), nullptr, 0, nullptr, 0, CXTranslationUnit_None);
	if (cxTU == nullptr) {
		std::cout << "ERROR: couldn't parse file " << fileName << std::endl;
	}
	else {

		CXCursor cxTUCursor = clang_getTranslationUnitCursor(cxTU);
		for (int i = 0; i < nas_getCursorChildCount(cxTUCursor); i++) {

			CXCursor cxChildCursor = nas_getCursorChild(cxTUCursor, i);
			if (clang_Location_isFromMainFile(clang_getCursorLocation(cxChildCursor))) {

				std::vector<std::string> globalVars;
				switch (clang_getCursorKind(cxChildCursor)) {

				case CXCursorKind::CXCursor_FunctionDecl:
				{
					FunctionData fData = { fileName, traverseFunction(cxChildCursor, globalVars) };
					functionMap->insert_or_assign(nas_getCursorSpelling(cxChildCursor), fData);
				}
				break;
				case CXCursorKind::CXCursor_VarDecl:
					if (std::find(globalVars.begin(), globalVars.end(), nas_getCursorUSR(cxChildCursor)) == globalVars.end()) {
						globalVars.push_back(nas_getCursorUSR(cxChildCursor));
					}
					break;
				}
			}
		}
	}
	clang_disposeTranslationUnit(cxTU);
	clang_disposeIndex(cxIndex);
}

void debugPrintGraphs(FunctionMap fMap) {
	for (std::pair<std::string, FunctionData> elem : fMap) {

		std::cout << elem.first << std::endl;
		int i = 0;
		for (GraphNodeData node : elem.second.dfGraph.nodeList) {

			std::cout << i << "{ ";
			for (int adjIndex : node.adjacency) {
				std::cout << adjIndex << " ";
			}
			std::cout << "}";
			if (elem.second.dfGraph.functionCallMap.find(i) != elem.second.dfGraph.functionCallMap.end()) {

				FunctionCallNode callNode = elem.second.dfGraph.functionCallMap.at(i);
				std::cout << " Called " << callNode.name << ", params ";
				for (Param param : callNode.params) {
					if (param.isRef) {
						std::cout << "*";
					}
					std::cout << "{ ";
					for (int paramNode : param.nodes) {
						std::cout << paramNode << " ";
					}
					std::cout << "}";
				}
			}
			std::cout << std::endl;
			i++;
		}
	}
}