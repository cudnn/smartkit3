#ifndef PARSED_RESULT__H
#define PARSED_RESULT__H

#include <iostream>
#include <string>
#include "ParsedResultType.h"

using namespace std;


class ParsedResult {
public:
//	ParsedResult(ParsedResultType type) {
//		this->type = type;
//	}
//	ParsedResult() {
//
//	}
	virtual string getDisplayResult() = 0;
public:
   ParsedResultType getType() {
	return type;
  }


public:
	ParsedResultType type;
	};
#endif


