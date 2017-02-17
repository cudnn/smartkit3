#ifndef WEBSITETYPE_HANDLER_H
#define WEBSITETYPE_HANDLER_H

#include <string>
#include <string>
#include "WebsiteType.h"
#include <vector>

class WebsiteTypeHandler {
public:
	WebsiteTypeHandler();
	WebsiteTypeList parser(std::string rowText);
	~WebsiteTypeHandler();


private:
	std::vector<WebsiteType*> mParser;
};


#endif
