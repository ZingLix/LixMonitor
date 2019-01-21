#include "User.h"
#include "boost/asio.hpp"
#include "msg.h"
#include "LogInfo.h"
#include "Component.h"
#include "openssl/sha.h"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <arpa/inet.h>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include  <cstdint>
#include <string>
using namespace boost::asio;
using namespace boost::beast;
User::User(socket_ptr&& soc_ptr, Server* s)
	:started(false),ws(false), soc_(std::move(soc_ptr)),
	read_buffer_(65536), write_buffer_(65536), server_(s)
{}

void User::start() {
	started = true;
	do_read();
}

void User::stop() {
	if (started == true) {
		started = false;
		LOG_INFO << socket()->remote_endpoint().address().to_string() << ":" << socket()->remote_endpoint().port() << " disconnected.";

		server_->visitor_close(shared_from_this());

	}
}

User::socket_ptr& User::socket() {
	return soc_;
}

void User::on_read(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		if (err == boost::asio::error::eof) {
		}
		stop();
		return;
	}
	std::string msg;
	{
		std::lock_guard<std::mutex> guard(read_mutex_);
		msg = std::string (&*read_buffer_.begin(), bytes);
	}
	if (msg.find("GET") != std::string::npos) {
		//	using namespace std::placeholders;
		ws_new(msg);
		//	ws_ = std::make_unique<websocket::stream<ip::tcp::socket>>(std::move(*soc_));
		//	soc_.reset();
		return;
	}
	std::vector<std::string> msg_set;
	msg_set = ws_read(msg);

	for (auto&messages : msg_set) {
		try {
			json_message message(messages);
			msg_exec(message);
		}
		catch (std::invalid_argument& e) {
			err_exec(1, e.what());
			stop();
		}
	}

}

const std::string base64_padding[] = { "", "==","=" };
std::string Base64Encode(const std::string& s)
{
	namespace bai = boost::archive::iterators;

	std::stringstream os;

	// convert binary values to base64 characters
	typedef bai::base64_from_binary
		// retrieve 6 bit integers from a sequence of 8 bit bytes
		<bai::transform_width<const char *, 6, 8> > base64_enc; // compose all the above operations in to a new iterator

	std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
		std::ostream_iterator<char>(os));

	os << base64_padding[s.size() % 3];
	return os.str();
}

void User::msg_exec(json_message& msg) {
    switch(msg.getInt("type")){
        case 1:
            return_Appinfo();
            break;
        default:
            break;
    }
}

void User::return_Appinfo(){
    auto s=server_->get_Appinfo();

    do_write(s);
}

void User::ws_new(std::string msg) {
	auto it = msg.find("Sec-WebSocket-Key: ");
	std::string key(msg.substr(it + 19, 43 - 19));
	key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	unsigned char hash[20];
	SHA_CTX s;
	SHA1_Init(&s);
	SHA1_Update(&s, key.c_str(), key.length());
	SHA1_Final(hash, &s);
	key = std::string(reinterpret_cast<char*>(hash), 20);

	key = Base64Encode(key);

	std::string response;
	response += "HTTP/1.1 101 Switching Protocols\r\n";
	response += "Connection: upgrade\r\n";
	response += "Sec-WebSocket-Accept: ";
	response += key + "\r\n";
	response += "Upgrade: websocket\r\n\r\n";
	ws = true;
	do_write(response);
}


void User::on_write(const boost::system::error_code& err, size_t bytes) {
	do_read();
}

void User::do_read() {
	if (started == false) return;
	using namespace std::placeholders;
	soc_->async_read_some(buffer(read_buffer_),
		std::bind(&User::on_read, this, _1, _2));
}

void User::do_write(const std::string& str) {
	if (started == false) return;
	if (ws == false) return;
	std::string s(ws_write(str));
	{
		std::lock_guard<std::mutex> lock_guard(write_mutex_);
		using namespace std::placeholders;
		std::copy(s.begin(), s.end(), &*write_buffer_.begin());
		soc_->async_write_some(buffer(write_buffer_, s.size()),
			std::bind(&User::on_write, this, _1, _2));
	}
}

std::string User::ws_write(const std::string& str) {
	std::string s;
	s += 0x81;
	if (str.length() <= 125) s += str.length();
	else if (str.length() <= 65535) {
		s += 126;
		std::uint16_t l = static_cast<std::uint16_t>(str.length());
		for (int i = 8; i >= 0; i -= 8) {
			auto tmp = l >> i;
			tmp &= 0x0ff;
			s += tmp;
		}
	}
	else {
		s += 127;
		std::uint16_t l = static_cast<std::uint16_t>(str.length());
		for (int i = 64 - 8; i >= 0; i -= 8) {
			auto tmp = l >> i;
			tmp &= 0x0ff;
			s += tmp;
		}
	}
	s += str;
	return s;
}


User::~User() {
	stop();
}

void User::err_exec(int code, const std::string& content) {
	json_message msg;
	msg.add("type", 9);
	msg.add("code", code);
	msg.add("content", content);
//	do_write(msg.getString());
}

std::vector<std::string> User::ws_read(std::string& str) {
	std::vector<std::string> msg_set;
	int pos = 0;
	while (pos != str.length()) {
		uint fin = (unsigned char)str[pos] >> 7;
		uint opcode_ = str[pos++] & 0x0f;
		uint mask = (unsigned char)str[pos] >> 7;

		uint payload_length_ = str[pos] & 0x7f;
		pos++;
		if (payload_length_ == 126) {
			uint16_t length = 0;
			memcpy(&length, str.c_str() + pos, 2);
			pos += 2;
			payload_length_ = ::ntohl(length);
		}
		else if (payload_length_ == 127) {
			uint64_t length = 0;
			memcpy(&length, str.c_str() + pos, 8);
			pos += 4;
			payload_length_ = ::ntohl(length);
		}

		std::array<char, 4> mask_key;
		for (int i = 0; i < 4; i++)
			mask_key[i] = str[pos + i];
		pos += 4;

		std::array<char, 1024> data;
		if (mask != 1) {
			std::copy(str.begin() + pos, str.end(), data.begin());
		}
		else {
			for (size_t i = 0; i < payload_length_; i++) {
				int j = i % 4;
				data[i] = str[pos + i] ^ mask_key[j];
			}
		}
		pos += payload_length_;
		msg_set.push_back(std::string(data.begin(), data.begin() + payload_length_));
	}
	return msg_set;
}

