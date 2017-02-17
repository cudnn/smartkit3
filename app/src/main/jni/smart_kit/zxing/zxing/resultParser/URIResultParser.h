#ifndef URIRESULTPARSER_H
#define URIRESULTPARSER_H

#include <iostream>
#include <string>
#include "ResultParser.h"
#include "URIParsedResult.h"

class URIResultParser : public ResultParser{
public:
	URIResultParser();
	virtual ~URIResultParser();
	void *parse(std::string rawText);

	bool isBasicallyValidURI(std::string uri) ;
private:
	URIParsedResult *_uri_parser_result;
};


#endif


