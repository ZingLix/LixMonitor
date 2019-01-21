#pragma once
#include "boost/asio.hpp"
#include <memory>
#include <vector>
#include "Server.h"
#include "msg.h"
#include <boost/beast.hpp>
#include <mutex>

class User:public std::enable_shared_from_this<User>
{
public:
	using socket_ptr = std::unique_ptr<boost::asio::ip::tcp::socket>;

	User(socket_ptr&& soc_ptr,Server* s);
	virtual ~User();
	void start();
	void stop();

	socket_ptr& socket();

	void on_write(const boost::system::error_code & err, size_t bytes);
	void on_read(const boost::system::error_code & err, size_t bytes);
	void do_read();
	void do_write(const std::string& str);

	void msg_exec(json_message& msg);

	void err_exec(int code, const std::string& content);
	void ws_new(std::string);
	std::vector<std::string> ws_read(std::string&);
	std::string ws_write(const std::string&);
    void return_Appinfo();
private:
	bool started;
	bool ws;
	std::unique_ptr<boost::asio::ip::tcp::socket> soc_;
	std::mutex read_mutex_;
	std::mutex write_mutex_;
	std::vector<char> read_buffer_;
	std::vector<char> write_buffer_;
	Server *server_;
};
