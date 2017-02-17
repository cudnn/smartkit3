#include <iostream>
#include <vector>
using namespace std;

namespace strTool
{
	string trim(const string& str)
	{
		string::size_type pos = str.find_first_not_of(' ');
		if (pos == string::npos)
		{
			return str;
		}
		string::size_type pos2 = str.find_last_not_of(' ');
		if (pos2 != string::npos)
		{
			return str.substr(pos, pos2 - pos + 1);
		}
		return str.substr(pos);
	}

	int split(const string& str, vector<string>& ret_, string sep = ",")
	{
		if (str.empty())
		{
			return 0;
		}

		string tmp;
		string::size_type pos_begin = str.find_first_not_of(sep);
		string::size_type comma_pos = 0;

		while (pos_begin != string::npos)
		{
			comma_pos = str.find(sep, pos_begin);
			if (comma_pos != string::npos)
			{
				tmp = str.substr(pos_begin, comma_pos - pos_begin);
				pos_begin = comma_pos + sep.length();
			}
			else
			{
				tmp = str.substr(pos_begin);
				pos_begin = comma_pos;
			}

			if (!tmp.empty())
			{
				ret_.push_back(tmp);
				tmp.clear();
			}
		}
		return 0;
	}
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
	bool contains(string str, string subs) {
		//int a = str.find(subs);
		//if (str.find(subs) == -1)
		//	return false;
		//else 
		//	return true;
		//int d = 1;
		return (str.find(subs) == -1) ? false : true;
	}

	string replace(const string& str, const string& src, const string& dest)
	{
		string ret;

		string::size_type pos_begin = 0;
		string::size_type pos = str.find(src);
		while (pos != string::npos)
		{
			cout << "replacexxx:" << pos_begin << " " << pos << "\n";
			ret.append(str.data() + pos_begin, pos - pos_begin);
			ret += dest;
			pos_begin = pos + 1;
			pos = str.find(src, pos_begin);
		}
		if (pos_begin < str.length())
		{
			ret.append(str.begin() + pos_begin, str.end());
		}
		return ret;
	}
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
	bool startsWith(string str, string prefix)
	{

		if (strncmp(str.c_str(), prefix.c_str(), prefix.length()) == 0)
		{
			return true;
		}
		return false;
	}

}
