#ifndef URIPASEREDRESULT_H
#define URIPASEREDRESULT_H

#include <iostream>
#include <string>
#include "ParsedResultType.h"
#include "WebsiteType.h"
#include "WebsiteTypeHandle.h"
#include "ParsedResult.h"
#include "strTool.h"
using namespace std;

class URIParsedResult : public ParsedResult{

private:
	string uri;
	string title;
	WebsiteTypeList websiteType;
	WebsiteTypeHandler webTypeParser;


public:
	std::string getDisplayResult(){return uri;}
	URIParsedResult(string uri, string title)
	{
		this->type = ParsedResultType_URI;
		this->uri = massageURI(uri);
		this->title = title;
		websiteType = WebsiteType_nonmatched;
	}
	string getURI() {
		return uri;
	}

	string getTitle() {
		return title;
	}
	WebsiteTypeList getWebsiteType() {
//		uri ="http://baike.qq.com.abc";
//		uri ="http://weixin.ss.com.abc";
//		uri ="http://www.baike.com.bddd";
		websiteType = webTypeParser.parser(uri);
		return websiteType;
	}




	/**
	* Transforms a string that represents a URI into something more proper, by adding or canonicalizing
	* the protocol.
	*/
	string massageURI(string uri) {
		//uri = uri.trim();
		uri = strTool::trim(uri);
		//int protocolEnd = uri.indexOf(':');
		int protocolEnd = uri.find_first_of(':');
		//if (protocolEnd < 0) {
		//	// No protocol, assume http
		//	uri = "http://" + uri;
		//}
		//else if (isColonFollowedByPortNumber(uri, protocolEnd)) {
		//	// Found a colon, but it looks like it is after the host, so the protocol is still missing
		//	uri = "http://" + uri;
		//}
		return uri;
	}

	// bool isColonFollowedByPortNumber(string uri, int protocolEnd) {
	//	int start = protocolEnd + 1;
	//	//int nextSlash = uri.indexOf('/', start);
	//	int nextSlash = uri.find_first_of(':', start);
	//	if (nextSlash < 0) {
	//		nextSlash = uri.length();
	//	}
	//	return ResultParser.isSubstringOfDigits(uri, start, nextSlash - start);
	//}

};

#endif

