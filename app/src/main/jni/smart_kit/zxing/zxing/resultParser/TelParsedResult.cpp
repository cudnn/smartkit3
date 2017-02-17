#include <iostream>
#include <string>
#include <zxing/resultParser/ParsedResultType.h>
#include <zxing/resultParser/ParsedResult.h>
#include <zxing/resultParser/strTool.h>
#include "TelParsedResult.h"

TelParsedResult::TelParsedResult(std::string number, std::string telURI, std::string title)
{
	this->type = ParsedResultType_TEL;
	this->number = number;
	this->telURI = telURI;
	this->title = title;
}
TelParsedResult::~TelParsedResult()
{
}
std::string TelParsedResult::getNumber()
{
	return number;
}
std::string TelParsedResult::getTelURI()
{
	return telURI;
}
std::string TelParsedResult::getTitle()
{
	return title;
}

