#include "ReportPrinter.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h> 

void printEventData(Event event, std::ostream& stream) {
	stream << "inside file " << event.file << ", function " << event.function << ", line " << event.line << ", col " << event.column;
}

void printEvents(std::vector<Event> eventList, std::ostream& stream) {
	
	for (Event event : eventList) {

		switch (event.type) {

		case EventType::source_detected:
			stream << std::left << std::setw(10) << "Info: " << std::setw(50) << "Source detected! ";
			break;
		case EventType::taint_to_sink:
			stream << std::left << std::setw(10) << "Warning: " << std::setw(50) << "Sink tainted! ";
			break;
		case EventType::taint_to_sanitizer:
			stream << std::left << std::setw(10) << "Info: " << std::setw(50) << "Taint sanitized! ";
			break;
		case EventType::taint_to_reference:
			stream << std::left << std::setw(10) << "Warning: " << std::setw(50) << "Referenced value potentially tainted. ";
			break;
		case EventType::taint_to_call:
			stream << std::left << std::setw(10) << "Warning: " << std::setw(50) << "Function called with tainted parameters. ";
			break;
		default:
			break;
		}
		printEventData(event, stream);
		stream << std::endl;
	}
}

void printReport(std::vector<Event> eventList, std::string outputFileName) {

	std::time_t now = time(0);
	if (!eventList.empty()) {
		if (!outputFileName.empty()) {

			std::ofstream stream{ outputFileName };
			if (!stream) {
				std::cout << "ERROR: couldn't open file for writing. The report will be printed in the command prompt." << std::endl;
				std::cout << "Report generated at " << ctime(&now) << std::endl;
				printEvents(eventList, std::cout);
			}
			else {
				stream << "Report generated " << ctime(&now) << std::endl;
				printEvents(eventList, stream);
			}
		}
		else {
			std::cout << "Report generated " << ctime(&now) << std::endl;
			printEvents(eventList, std::cout);
		}
	}
}