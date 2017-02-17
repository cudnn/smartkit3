//#include "ResultParserHandle.h"
//#include <regex>
//
#include <string>
#include "ResultParserHandle.h"
#include "URIResultParser.h"
#include "EmailResultParser.h"
#include "TelResultParser.h"
#include "ResultParser.h"
#include <vector>

ResultParserHandle::ResultParserHandle()
{
	ResultParser *temp = new URIResultParser();
	mParser.push_back(temp);
	temp = new EmailResultParser();
	mParser.push_back(temp);
	temp = new TelResultParser();
	mParser.push_back(temp);
}
ResultParserHandle::~ResultParserHandle()
{
	for(int i = 0; i < mParser.size(); i++)
	{
		delete mParser[i];
	}
}

ParsedResult* ResultParserHandle::parser(std::string rowText)
{
	for(int i = 0; i < mParser.size(); i++)
	{
		ResultParser *tempParser = mParser[i];
		ParsedResult *mParsedResult;
		mParsedResult = (ParsedResult*) (tempParser->parse(rowText));
		if(mParsedResult != NULL)
			return mParsedResult;
	}
	return NULL;
}

