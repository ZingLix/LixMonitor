#pragma once
#include <string>
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class json_message
{
public:
	json_message() :allocator(document.GetAllocator()) {
		document.SetObject();
	}

	json_message(std::string str) :allocator(document.GetAllocator()) {
		document.Parse(str.c_str());
		if (!document.IsObject()|| document.HasParseError()) {
			throw std::invalid_argument("Parse Error");
		}
	}

	//void add(std::string key, int val) {
	//	rapidjson::Value n(key.c_str(), allocator);
	//	document.AddMember(n, val, allocator);
	//}

	template<class T>
	void add(std::string key, T val) {
		rapidjson::Value n(key.c_str(), allocator);
		document.AddMember(n, val, allocator);
	}

	void add(std::string key, std::string val) {
		rapidjson::Value k(key.c_str(), allocator);
		rapidjson::Value v(val.c_str(), allocator);
		document.AddMember(k, v, allocator);
	}

	void add(std::string key, rapidjson::Value& val) {
		rapidjson::Value k(key.c_str(), allocator);
		document.AddMember(k, val, allocator);
	}

	void clear() {
		document.SetObject();
	}

	int getInt(const char* key) const {
		if (!document.HasMember(key)) throw std::invalid_argument("No such member.");
		return document[key].GetInt();
	}

	std::uint64_t getUInt64(const char* key) const {
		if (!document.HasMember(key)) throw std::invalid_argument("No such member.");
		return document[key].GetUint64();
	}

	std::string getString() const {
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<>> writer(buffer);
		document.Accept(writer);
		return buffer.GetString();
	}

	std::string getString(const char* key) const {
		if (!document.HasMember(key)) throw std::invalid_argument("No such member.");
		return document[key].GetString();
	}

	rapidjson::MemoryPoolAllocator<>& getAllocator() const {
		return allocator;
	}

	void add(std::string key,rapidjson::Value&& val) {
		rapidjson::Value k(key.c_str(), allocator);
		document.AddMember(k, val, allocator);
	}

	bool hasMember(std::string key) {
		return document.HasMember(key);
	}

private:
	rapidjson::Document document;
	rapidjson::MemoryPoolAllocator<>& allocator;
};
