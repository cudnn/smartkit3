#include <string>
#include <string>
#include "WebsiteTypeHandle.h"
#include "WebsiteType.h"
#include <vector>

WebsiteTypeHandler::WebsiteTypeHandler()
{
	WebsiteType *temp = new WeinXin();
	mParser.push_back(temp);
	temp = new Baidu();
	mParser.push_back(temp);
	temp = new BaiKe();
	mParser.push_back(temp);
}

WebsiteTypeHandler::~WebsiteTypeHandler()
{
	for(int i = 0; i < mParser.size(); i++)
	{
		delete mParser[i];
	}
}

WebsiteTypeList WebsiteTypeHandler::parser(std::string rowText)
{
	for(int i = 0; i < mParser.size(); i++)
	{
		if(mParser[i]->parser(rowText))
		{
			return mParser[i]->getType();
		}
	}
	return WebsiteType_nonmatched;
}



