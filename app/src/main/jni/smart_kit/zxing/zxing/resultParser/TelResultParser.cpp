//#include "TelResultParser.h"
#include "ResultParser.h"
#include "TelResultParser.h"
#include "TelParsedResult.h"
#include <string>
#include <iostream>
#include "strTool.h"

	TelResultParser::TelResultParser()
	{
		_tel_parser_result = NULL;
	}

	TelResultParser::~TelResultParser()
	{
		if(_tel_parser_result != NULL)
		{
			delete _tel_parser_result;
			_tel_parser_result = NULL;
		}
	}
void *TelResultParser::parse(string rawText)
{
	// We specifically handle the odd "URL" scheme here for simplicity and add "URI" for fun
	// Assume anything starting this way really means to be a URI
	if (!strTool::startsWith(rawText, "tel:") && !strTool::startsWith(rawText,"TEL:")) {
		_tel_parser_result = NULL;
		return _tel_parser_result;
	}
	// Normalize "TEL:" to "tel:"
	std::string telURI = strTool::startsWith(rawText,"TEL:") ? "tel:" + rawText.substr(4) : rawText;
	// Drop tel, query portion
	int queryStart = rawText.find('?', 4);
	std::string number =  queryStart < 0 ? rawText.substr(4) : rawText.substr(4, queryStart);

	_tel_parser_result = new TelParsedResult(number, telURI, "");
	return _tel_parser_result;
}


