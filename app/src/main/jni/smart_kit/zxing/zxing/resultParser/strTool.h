#ifndef STR_TOOL_H
#define STR_TOOL_H
#include <iostream>
#include <vector>
using namespace std;


namespace strTool
{
	string trim(const string& str);


	int split(const string& str, vector<string>& ret_, string sep = ",");

	/**
	* Determines if this {@code String} contains the sequence of characters in
	* the {@code CharSequence} passed.
	*
	* @param cs
	*            the character sequence to search for.
	* @return {@code true} if the sequence of characters are contained in this
	*         string, otherwise {@code false}.
	* @since 1.5
	*/
	bool contains(string str, string subs) ;

	string replace(const string& str, const string& src, const string& dest);

	/**
	* Compares the specified string to this string to determine if the
	* specified string is a prefix.
	*
	* @param prefix
	*            the string to look for.
	* @return {@code true} if the specified string is a prefix of this string,
	*         {@code false} otherwise
	* @throws NullPointerException
	*             if {@code prefix} is {@code null}.
	*/
	bool startsWith(string str, string prefix);


}

#endif


