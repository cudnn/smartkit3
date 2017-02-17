#ifndef TELRESULTPARSER_H
#define TELRESULTPARSER_H

#include <iostream>
#include <string>
#include "ResultParser.h"
#include "TelParsedResult.h"

class TelResultParser : public ResultParser{
public:
	TelResultParser();
	virtual ~TelResultParser();
	void *parse(std::string rawText);
private:
	TelParsedResult *_tel_parser_result;
};

#endif


