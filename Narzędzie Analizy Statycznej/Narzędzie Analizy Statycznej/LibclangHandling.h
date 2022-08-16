#ifndef NAS_LIBCLANG_HANDLING
#define NAS_LIBCLANG_HANDLING

#include "clang-c/Index.h"

#include <iostream>
#include <functional>

using NestedVisitorData = struct {

    const std::function<CXChildVisitResult(CXCursor, void*)> lambda;
    void* data;
};

std::string nas_getCursorSpelling(CXCursor cxCursor);

std::string nas_getCursorUSR(CXCursor cxCursor);

std::string nas_getCursorTypeKind(CXCursor cxCursor);

CXChildVisitResult nas_nestedVisitor(CXCursor cxCursor, CXCursor cxParent, CXClientData cxClientData);

bool nas_cursorIsAssignmentOperator(CXCursor cxCursor);

int nas_getCursorChildCount(CXCursor cxCursor);

CXCursor nas_getCursorChild(CXCursor cxCursor, int childIndex);

#endif
