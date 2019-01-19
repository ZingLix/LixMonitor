#pragma once
#include <string>

struct item
{
	item(std::uint64_t id,std::string name,std::string info,std::string location)
		:item_id(id),item_name(name),item_info(info),lost_location(location)
	{}

	std::uint64_t item_id;
	std::string item_name;
	std::string item_info;
	std::string lost_location;
};


struct userinfo
{
	userinfo(std::uint64_t id,std::string username_, std::string email_, std::string phone_, std::string description_)
		:user_id(id),username(username_), email(email_),phone(phone_),description(description_)
	{}

	std::uint64_t user_id;
	std::string username;
	std::string email;
	std::string phone;
	std::string description;
};

struct message
{
	message(std::uint64_t id, std::uint64_t senderid, std::uint64_t recverid, std::string content_)
		:msg_seq_id(id),sender_id(senderid),recver_id(recverid),content(content_)
	{}

	std::uint64_t msg_seq_id;
	std::uint64_t sender_id;
	std::uint64_t recver_id;
	std::string content;
};