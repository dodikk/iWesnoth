/*
 Copyright (C) 2009 by Kyle Poole <kyle.poole@gmail.com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */

#ifndef SHARED_STRING_HPP_INCLUDED
#define SHARED_STRING_HPP_INCLUDED

#include <string>
#include <map>

// do the strings get deleted when not used, or just kept around? performance vs memory
#define DONT_CLEAR_STRINGS

struct string_node
{
	string_node(const std::string &aStr)
	{
		str = aStr;
		count = 1;
	}
	
	std::string str;
	unsigned long count;
};

class shared_string
{
public:
	shared_string(const shared_string &o);
	shared_string(const std::string &o);	
	shared_string(const char* o);
	shared_string();
	
	~shared_string();
	
	operator const std::string&() const
	{
		assert(data_);
		return data_->str;
	}

	const char* c_str() const {
		assert(data_);
		return data_->str.c_str();
	}
	
	
	bool empty() const;
	
	size_t find(const std::string& str, size_t pos = 0) const;	
	std::string substr(size_t pos = 0, size_t n = std::string::npos) const;
	
	size_t size() const;
	size_t length() const
	{
		return size();
	}
	
	std::string::const_iterator begin() const;	
	std::string::const_iterator end() const;
	
	shared_string& operator =(const shared_string &other);
	shared_string& operator =(const char *other);
	shared_string& operator =(const std::string &other);
	
	bool operator !=(const shared_string& str) const
	{
		return strcmp(c_str(), str.c_str()) != 0;
	}
	
	bool operator !=(const char *str) const
	{
		return strcmp(c_str(), str) != 0;
	}
	
	bool operator !=(const std::string& str) const
	{
		return strcmp(c_str(), str.c_str()) != 0;
	}
	
	bool operator ==(const char *str) const
	{
		return strcmp(c_str(), str) == 0;
	}
	
	bool operator ==(const std::string& str) const
	{
		return strcmp(c_str(), str.c_str()) == 0;
	}
	
	bool operator ==(const shared_string& str) const
	{
		return strcmp(c_str(), str.c_str()) == 0;
	}
	
	bool operator <(const shared_string& b) const
	{
		return data_->str < b.data_->str;
	}
	
	char operator [] (int pos) const
	{
		return data_->str[pos];
	}
	
	
	shared_string& operator +=(const shared_string& str)
	{
		std::string temp = data_->str + str.data_->str;
		clear();
		set(temp);
		return *this;
	}
	
	size_t find(char a) const
	{
		return data_->str.find(a);
	}
	
	size_t find_last_of(char c) const
	{
		return data_->str.find_last_of(c);
	}
	
	size_t find_first_not_of(const char *str, size_t pos) const
	{
		return data_->str.find_first_not_of(str, pos);
	}

	size_t find_last_not_of(char c) const
	{
		return data_->str.find_last_not_of(c);
	}
	
	void assign(const char *str, size_t size)
	{
		std::string temp(str,size);
		clear();
		set(temp);
	}
	
	void resize(size_t newSize)
	{
		std::string temp(data_->str);
		temp.resize(newSize);
		clear();
		set(temp);
	}
	
	int compare(int start, int end, std::string &cmpStr) const
	{
		return get().compare(start, end, cmpStr);
	}

	int compare(int start, int end, const char *cmpStr) const
	{
		return get().compare(start, end, cmpStr);
	}
	
	const std::string& get() const { assert(data_); return data_->str; }

	void clear();
	void set(const std::string &str);

	
	
	string_node *data_;

};

/*
inline std::string& operator +(std::string& lhs, const shared_string rhs)
{
	lhs.append(rhs.get());
	return lhs;
}
*/

// "abc" + shared_string
inline std::string operator +(const char* lhs, const shared_string& rhs)
{
	std::string ret(lhs);
	ret.append(rhs.c_str());
	return ret;
}

// 'a' + shared_string
inline std::string operator +(const char lhs, const shared_string& rhs)
{
	std::string ret;
	ret.append(1,lhs);
	ret.append(rhs.c_str());
	return ret;
}

inline std::string operator +(const std::string& lhs, const shared_string& rhs)
{
	std::string ret(lhs);
	ret.append(rhs.c_str());
	return ret;
}

inline bool operator ==(const std::string& lhs, const shared_string& rhs)
{
	return strcmp(lhs.c_str(), rhs.c_str()) == 0;
}

inline bool operator !=(const std::string& lhs, const shared_string& rhs)
{
	return strcmp(lhs.c_str(), rhs.c_str()) != 0;
}

inline size_t hash_value(const shared_string& str) 
{
	return hash_value(std::string(str.c_str()));
}

inline std::ostream& operator<<(std::ostream& stream, const shared_string& o) {
	return stream << o.get();
}

inline std::istream& operator>>(std::istream& stream, shared_string& o) {
	std::string str;
	std::istream& ret = stream >> str;
	o.clear();
	o.set(str);
	return ret;
}

size_t shared_count(const std::string &str);
#endif

