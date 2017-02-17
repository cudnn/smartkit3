#ifndef EMAIL_RESULT_PARSER_H
#define EMAIL_RESULT_PARSER_H

#include "regex.h"

#include <iostream>

#include <string>
#include <iostream>
#include "ResultParser.h"
#include "EmailParsedResult.h"
#include "strTool.h"

class EmailResultParser : public ResultParser {
public:
	EmailResultParser()
	{

	}
	~EmailResultParser()
	{

	}

	void *parse(string rawText) {
		if (strTool::startsWith(rawText, "URL:")
				|| strTool::startsWith(rawText, "URI:")) {

			return new EmailParsedResult(strTool::trim(rawText.substr(4)), NULL);
		}
		rawText = strTool::trim(rawText);
//		EmailParsedResult *a = new EmailParsedResult(rawText, "");
		return (void*) (
				isBasicallyValidEmail(rawText) ?
						new EmailParsedResult(rawText, "") : NULL);

	}

	bool isBasicallyValidEmail(string uri) {
		if (strTool::contains(uri, " ")) {
			//	// Quick hack check for a common case
			return false;
		}
		//"(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"
		const char *pattern =
				//"([a-zA-Z0-9\\-]+\\.)+[a-zA-Z]{2,}(:[0-9]{1,5})?(/|\\?|$)";
//				"^([a-zA-Z])+@[a-zA-Z0-9]+(/|\\?|$)";
				"^[a-z]([a-z0-9]*[-_\\.]?[a-z0-9]+)*@([a-z0-9]*[-_]?[a-z0-9]+)+[\\.][a-z]{2,3}([\\.][a-z]{2})?$";
				//"[[.ch.]]*c";
//				"nfa|cfd.not";

		regex_t URL_WITH_PROTOCOL_PATTERN;
		regmatch_t pmatch[5];

		int ret = regcomp(&URL_WITH_PROTOCOL_PATTERN, pattern, REG_EXTENDED); // 编译正则表达式，为reg分配内存
	//	assert(ret == 0);

		int err = regexec(&URL_WITH_PROTOCOL_PATTERN, uri.c_str(), 5, pmatch, 0); // 匹配正则表达式
		if (err == REG_NOMATCH) {
			return false;
		} else if (err) {
			return false;
		}
		if (pmatch[0].rm_so == 0)
			return true;
		else
			return false;
	}
};

#endif

