#include "libclanghandling.h"

std::string nas_getCursorSpelling(CXCursor cxCursor) {

    CXString cxCursorSpelling = clang_getCursorSpelling(cxCursor);
    std::string result = clang_getCString(cxCursorSpelling);

    clang_disposeString(cxCursorSpelling);
    return result;
}

std::string nas_getCursorUSR(CXCursor cxCursor) {

    std::string result;
    if (clang_isDeclaration(clang_getCursorKind(cxCursor)))
    {
        CXString cursorSpelling = clang_getCursorUSR(cxCursor);
        result = clang_getCString(cursorSpelling);
        clang_disposeString(cursorSpelling);
    }
    else if (!clang_Cursor_isNull(clang_getCursorReferenced(cxCursor)))
    {
        CXString cursorSpelling = clang_getCursorUSR(clang_getCursorReferenced(cxCursor));
        result = clang_getCString(cursorSpelling);
        clang_disposeString(cursorSpelling);
    }
    return result;
}

std::string nas_getCursorTypeKind(CXCursor cxCursor) {

    CXString kindName = clang_getTypeKindSpelling(clang_getCursorType(cxCursor).kind);
    std::string result = clang_getCString(kindName);

    clang_disposeString(kindName);
    return result;
}

CXChildVisitResult nas_nestedVisitor(CXCursor cxChildCursor, CXCursor cxParentCursor, CXClientData cxClientData) {

    NestedVisitorData visitorData = *(reinterpret_cast<NestedVisitorData*>(cxClientData));
    CXChildVisitResult result = visitorData.lambda(cxChildCursor, visitorData.data);

    if (result == CXChildVisit_Continue) {

        clang_visitChildren(cxChildCursor, nas_nestedVisitor, cxClientData);
        return result;
    }
    else
        return result;
}

bool nas_cursorIsAssignmentOperator(CXCursor cxCursor) {

    const CXTranslationUnit cxTU = clang_Cursor_getTranslationUnit(cxCursor);
    if (clang_getCursorKind(cxCursor) == CXCursorKind::CXCursor_BinaryOperator) {

        CXSourceLocation cxLocation = clang_getCursorLocation(cxCursor);
        clang_visitChildren(
            cxCursor,
            [](CXCursor cxChildCursor, CXCursor cxParentCursor, CXClientData cxClientData) {

                CXSourceLocation* visitor_cxLocation = reinterpret_cast<CXSourceLocation*>(cxClientData);
                CXSourceRange cxRange = clang_getCursorExtent(cxChildCursor);
                visitor_cxLocation->int_data = cxRange.end_int_data;
                return CXChildVisit_Break;
            },
            &cxLocation
        );
        CXToken cxToken = *clang_getToken(cxTU, cxLocation);
        CXString cxTokenSpelling = clang_getTokenSpelling(cxTU, cxToken);
        std::string cursorSpelling = clang_getCString(cxTokenSpelling);
        clang_disposeString(cxTokenSpelling);
        if (cursorSpelling == "=")
            return true;
        else return false;
    }
    else return false;
}

int nas_getCursorChildCount(CXCursor cxCursor) {

    int childCount = 0;
    clang_visitChildren(
        cxCursor,
        [](CXCursor cxParentCursor, CXCursor cxChildCursor, CXClientData cxClientData) {

            int* visitor_count = reinterpret_cast<int*>(cxClientData);
            (*visitor_count)++;
            return CXChildVisit_Continue;
        },
        & childCount
    );
    return childCount;
}

CXCursor nas_getCursorChild(CXCursor cxCursor, int childIndex) {

    int i = 0;
    CXCursor cxExtractedCursor = clang_getNullCursor();
    using Package = struct { int childIndex; int* i; CXCursor* cxExtractedCursor; };
    Package package = { childIndex, &i, &cxExtractedCursor };
    clang_visitChildren(
        cxCursor,
        [](CXCursor cxCurrentCursor, CXCursor cxParentCursor, CXClientData cxClientData) {

            Package* visitor = reinterpret_cast<Package*>(cxClientData);
            if (*(visitor->i) == visitor->childIndex) {
                
                *(visitor->cxExtractedCursor) = cxCurrentCursor;
                return CXChildVisit_Break;
            }
            (*visitor->i)++;
            return CXChildVisit_Continue;
        },
        &package
    );
    return cxExtractedCursor;
}