#ifndef NAS_REPORT_PRINTER
#define NAS_REPORT_PRINTER

#include "TaintTracking.h"

#include <iostream>

void printReport(std::vector<Event> eventList, std::string outputFileName);

#endif
