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


#include "shared_string.hpp"

#include <map>
//#include "skiplist_map.hpp"
//#include <boost/unordered_map.hpp>
//#include "AssocVector.h"

class string_compare
{
	public:
		string_compare(void)
		{
		}
	
		bool operator()(const std::string *left, const std::string *right) const
		{				
			return (left->compare(*right) < 0);
		}
};

class string_hasher
{
	public:
		string_hasher(void)
		{
		}
	
	unsigned long operator()(const std::string *aStr) const
	{
		const char *str = aStr->c_str();
		unsigned long hash = 5381;
		int c;
		
		while (c = *str++)
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		
		return hash;
	}
};


typedef std::map<const std::string*, string_node*, string_compare> MAP;
//typedef skiplist_map<const std::string*, string_node*, string_compare> MAP;
//typedef boost::unordered_map<const std::string*, string_node*, string_hasher> MAP;
//typedef AssocVector<const std::string*, string_node*, string_compare> MAP;
typedef MAP::iterator ITERATOR;
typedef std::pair<const std::string*, string_node*> PAIR;

string_node* gNullData;
MAP* gMap = NULL;

// this is just to make sure that the static map is initialized, since shared_strings might be static vars somewhere
MAP& gStringPool() 
{ 
	//static MAP* map = new MAP; 
	if (gMap == NULL)
		gMap = new MAP;
	return *gMap; 
}


void shared_string::set(const std::string &str)
{	
	ITERATOR fnd;
	/*
	tmp.second = new string_node(str);
	tmp.first = &tmp.second->str;
	std::pair<ITERATOR, bool> res = gStringPool().insert(tmp);
	if (res.second == false)
	{
		// it already existed...
		assert(tmp.first != res.first->first);
		delete tmp.second;
		++res.first->second->count;
	}
	data_ = res.first->second;
	assert(data_);
	 */
	
	// optimize common case of "" string
	if (str.length() == 0 && gNullData)
	{
		data_ = gNullData;
		data_->count++;
		return;
	}
	
	fnd = gStringPool().find(&str);
	if (fnd == gStringPool().end())
	{
		PAIR tmp;
		tmp.second = new string_node(str);
		tmp.first = &tmp.second->str;
		gStringPool().insert(tmp);
		data_ = tmp.second;
	}
	else
	{
		++fnd->second->count;
		data_ = fnd->second;
	}
	assert(data_);
}

shared_string::shared_string()
{
	// optimize common case of "" string
	if (gNullData)
	{
		data_ = gNullData;
		data_->count++;
	}
	else
	{
		set("");
		gNullData = data_;
		data_->count++;
	}
}

shared_string::shared_string(const std::string& o)
{
	set(o);
}

shared_string::shared_string(const char *o)
{
	assert(o);
	set(o);
}

shared_string::shared_string(const shared_string &o)
{
	assert(o.data_);
		
	data_ = o.data_;
	data_->count++;
	
	return;
}

shared_string& shared_string::operator =(const shared_string &other)
{
	if (&other == this)
		return *this;

	assert(data_);
	//assert(other.data_);
	clear();
	if (!other.data_)
	{
		set("");
	}
	else
	{
		//set(other.get());
		data_ = other.data_;
		data_->count++;
	}
	return *this;
}

shared_string& shared_string::operator =(const char *other)
{
	assert(other);
	clear();
	set(other);
	return *this;
}

shared_string& shared_string::operator =(const std::string &other)
{
	clear();
	set(other);
	return *this;
}


shared_string::~shared_string()
{
	assert(data_);
	clear();
}

void shared_string::clear()
{	
	assert(data_);
	if (data_ != NULL)
	{
		if (--data_->count <= 0)
		{
#ifndef DONT_CLEAR_STRINGS
			ITERATOR fnd;
			fnd = gStringPool().find(&data_->str);
			gStringPool().erase(fnd);
			delete data_;
#endif
		}
	}
	
	data_ = NULL;
}

bool shared_string::empty() const
{
	if (!data_)
		return true;
	
	return data_->str.empty();
}

size_t shared_string::find(const std::string& str, size_t pos) const
{
	return data_->str.find(str, pos);
}

std::string shared_string::substr(size_t pos, size_t n) const
{
	return data_->str.substr(pos, n);
}

size_t shared_string::size() const
{
	return data_->str.size();
}

std::string::const_iterator shared_string::begin() const
{
	return data_->str.begin();
}

std::string::const_iterator shared_string::end() const
{
	return data_->str.end();
}

size_t shared_count(const std::string &str)
{
	ITERATOR fnd;
	fnd = gStringPool().find(&str);
	if (fnd == gStringPool().end())
		return 0;
	
	return fnd->second->count;
}

