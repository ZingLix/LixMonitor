#pragma once
#include "boost/asio.hpp"

class InfoGenerator;
class User;

class Server
{
public:
	using err_code = boost::system::error_code;
	Server(unsigned short port);
	Server(boost::asio::ip::tcp::endpoint);
	void start(const InfoGenerator& g);

	boost::asio::ip::tcp::acceptor& acceptor() { return acceptor_; }

	void handle_accept(const err_code& ec);

	void visitor_close(const std::shared_ptr<User>& use_ptr);

	void send(const InfoGenerator& g);

private:
	void updateAcceptor();

	boost::asio::io_context context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	std::unique_ptr<boost::asio::ip::tcp::socket> waiting_soc_;
	std::vector<std::shared_ptr<User>> vistor_list_;
};