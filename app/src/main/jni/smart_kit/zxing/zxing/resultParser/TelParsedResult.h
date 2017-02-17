#ifndef TELPARSEDRESULT_H
#define TELPARSEDRESULT_H
//#pragma once
#include <iostream>
#include <string>
#include "ParsedResultType.h"
#include "ParsedResult.h"
#include "strTool.h"
class TelParsedResult : public ParsedResult{

public:
	TelParsedResult(std::string number, std::string telURI, std::string title);
	virtual ~TelParsedResult();
	std::string getNumber();
    std::string getTelURI();
    std::string getTitle();
    std::string getDisplayResult(){return number;}
private:
	  std::string number;
	  std::string telURI;
	  std::string title;
};
#endif

