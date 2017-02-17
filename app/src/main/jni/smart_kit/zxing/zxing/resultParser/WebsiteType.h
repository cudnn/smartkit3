#ifndef WEBSITETYPE_H
#define WEBSITETYPE_H


#include <string>
#include <iostream>
#include "strTool.h"

#ifdef REGEX
#include "regex.h"
#endif

enum WebsiteTypeList{
	WebsiteType_wikipedia,
	WebsiteType_weibo,
	WebsiteType_weixin,
	WebsiteType_baidu,
	WebsiteType_baike,
	WebsiteType_nonmatched
};


class WebsiteType
{
public:
	//
	WebsiteType()
    {
		webtypeId = WebsiteType_nonmatched;
    }
	bool parser(std::string str)
	{
#ifdef REGEX
		int ret = regcomp(&URL_PATTERN,pattern,REG_EXTENDED);  // 编译正则表达式，为reg分配内存
		int err = regexec(&URL_PATTERN,str.c_str(),5,pmatch,0);  // 匹配正则表达式
		if(err == REG_NOMATCH)
		{
			return false;
		}
		else if(err)
		{
			return false;
		}
		if(pmatch[0].rm_so == 0)
			return true;
		else
			return false;
#else
		return strTool::startsWith(str, pattern);
#endif
	}

	WebsiteTypeList getType()
	{
		return webtypeId;
	}

public:
	WebsiteTypeList webtypeId;
	const char *pattern;
#ifdef REGEX
	regex_t URL_PATTERN;
	regmatch_t pmatch[5];
#endif
};

class WeinXin:public WebsiteType{
public:
	WeinXin()   {
#ifdef REGEX
		pattern = "^(http://weixin.)([a-zA-Z0-9\\-]+\\.)+[a-zA-Z]{2,}(/|\\?|$)";
#else
		pattern = "http://weixin.";
#endif
//		pattern = "^(http://weixin.qq.)([a-zA-Z0-9\\-]+\\.)+[a-zA-Z]{2,}(/|\\?|$)";
		webtypeId = WebsiteType_weibo;
   }
};

class Baidu:public WebsiteType{
public:
	Baidu(){
#ifdef REGEX
		pattern =  "^(http://www.baidu.)([a-zA-Z0-9\\-]+\\.)+[a-zA-Z]{2,}(:[0-9]{1,5})?(/|\\?|$)";
#else
		pattern = "http://www.baidu.";
#endif
		webtypeId = WebsiteType_baidu;
   }
};
class BaiKe:public WebsiteType{
public:
	BaiKe(){
#ifdef REGEX
		pattern = "^(http://baike.)([a-zA-Z0-9\\-]+\\.)+[a-zA-Z]{2,}(/|\\?|$)";
#else
		pattern = "http://baike.";
#endif
		webtypeId = WebsiteType_baike;
   }
};

#endif



