#ifndef RESULTPASRERHADLERL_H
#define RESULTPASRERHADLERL_H

#include <string>
#include <string>
#include "URIResultParser.h"
#include "EmailResultParser.h"
#include "ResultParser.h"
#include <vector>

class ResultParserHandle {
public:
	ResultParserHandle();
//	{
//		ResultParser *temp = new URIResultParser();
//		mParser.push_back(temp);
//		temp = new EmailResultParser();
//		mParser.push_back(temp);
//	}

	ParsedResult* parser(std::string rowText);
//	{
//		for(int i = 0; i < mParser.size(); i++)
//		{
//			ResultParser *tempParser = mParser[i];
//			ParsedResult *mParsedResult;
//			mParsedResult = (ParsedResult*) (tempParser->parse(rowText));
//			if(mParsedResult != NULL)
//				return mParsedResult;
//		}
//		return NULL;
//	}
	~ResultParserHandle();
//	{
//		for(int i = 0; i < mParser.size(); i++)
//		{
//			delete mParser[i];
//		}
//	}

private:
	std::vector<ResultParser*> mParser;
};
#endif
