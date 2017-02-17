#ifndef EMAIL_PARSED_RESULT_H
#define EMAIL_PARSED_RESULT_H

#include <iostream>
#include <string>
#include "ParsedResultType.h"
#include "ParsedResult.h"
#include "strTool.h"
using namespace std;
class EmailParsedResult : public ParsedResult{

private:
	string uri;
	string title;

public:
	EmailParsedResult(string uri, string title);
	~EmailParsedResult();
	string getURI() ;

	string getTitle() ;

	/**
	* Transforms a string that represents a URI into something more proper, by adding or canonicalizing
	* the protocol.
	*/
	string massageURI(string uri) ;
	string getDisplayResult(){return title;}
};

#endif


