#include "URIResultParser.h"
#include "regex.h"
#include <string>
#include <iostream>
#include "ResultParser.h"
#include "URIParsedResult.h"
#include "strTool.h"

	URIResultParser::URIResultParser()
	{
		_uri_parser_result = NULL;
	}

	URIResultParser::~URIResultParser()
	{
		if(_uri_parser_result != NULL)
		{
			delete _uri_parser_result;
			_uri_parser_result = NULL;
		}
	}
	void *URIResultParser::parse(string rawText)
	{
		// We specifically handle the odd "URL" scheme here for simplicity and add "URI" for fun
		// Assume anything starting this way really means to be a URI
		if (strTool::startsWith(rawText, "URL:") || strTool::startsWith(rawText,"URI:")) {

			_uri_parser_result = new URIParsedResult(strTool::trim(rawText.substr(4)), NULL);
			return _uri_parser_result;
		}
		rawText = strTool::trim(rawText);
//		URIParsedResult *a = new URIParsedResult(rawText, "");
		if(isBasicallyValidURI(rawText)) {
			_uri_parser_result = new URIParsedResult(rawText, "");
		}
		else {
			_uri_parser_result = NULL;
		}

		return _uri_parser_result;
	}

	 bool URIResultParser::isBasicallyValidURI(string uri) {
		 if (strTool::contains(uri, " ")) {
		//	// Quick hack check for a common case
			return false;
		}
//		   const char *pattern = "([http|https]://)?([/w-]+/.)+[/w-]+(/[/w- ./?%&amp;=]*)?";
//		   const char *pattern = "([http|https]://)?([/w-]+/.)+[a-zA-Z]{2,}(:\\d{1,5})?(/|\\?|$)";

		   const char *pattern = "^[a-zA-Z]{3,5}(://)([a-zA-Z0-9\\-]+\\.)+[a-zA-Z]{2,}(:[0-9]{1,5})?(/|\\?|$)";

		    regex_t URL_WITH_PROTOCOL_PATTERN;
		    regmatch_t pmatch[5];

		    int ret = regcomp(&URL_WITH_PROTOCOL_PATTERN,pattern,REG_EXTENDED);  // 编译正则表达式，为reg分配内存
//		    assert(ret==0);

		    int err = regexec(&URL_WITH_PROTOCOL_PATTERN,uri.c_str(),5,pmatch,0);  // 匹配正则表达式
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
    	}
