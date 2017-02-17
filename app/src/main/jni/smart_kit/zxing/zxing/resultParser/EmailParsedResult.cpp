#include "EmailParsedResult.h"
#include <iostream>
#include <string>
#include "ParsedResultType.h"
#include "ParsedResult.h"
#include "strTool.h"
using namespace std;
	EmailParsedResult::EmailParsedResult(string uri, string title)
	{
		//ParsedResult(ParsedResultType_EMAIL_ADDRESS);
		this->type = ParsedResultType_EMAIL_ADDRESS;
		this->uri = massageURI(uri);
		this->title = title;
	}
	EmailParsedResult::~EmailParsedResult()
	{

	}
	string EmailParsedResult::getURI() {
		return uri;
	}

	string EmailParsedResult::getTitle() {
		return title;
	}

	/**
	* Transforms a string that represents a URI into something more proper, by adding or canonicalizing
	* the protocol.
	*/
	string EmailParsedResult::massageURI(string uri) {
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

//};
