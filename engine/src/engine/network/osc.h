#ifndef __OSC_H__
#define __OSC_H__

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class Osc
{
	private:
		udp::socket _Socket;
		udp::endpoint _SenderEndpoint;
		boost::asio::io_service _IoService;

		int _ComPort = 8001;
		enum { max_length = 1024 };
		char data_[max_length];

public:
	Osc(boost::asio::io_service & _IoService, short port)
		: _Socket(_IoService, udp::endpoint(udp::v4(), port))
	{
		_Socket.async_receive_from(
			boost::asio::buffer(data_, max_length), _SenderEndpoint,
			boost::bind(&Osc::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive_from(const boost::system::error_code& error,
		size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
			_Socket.async_send_to(
				boost::asio::buffer(data_, bytes_recvd), _SenderEndpoint,
				boost::bind(&Osc::handle_send_to, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			_Socket.async_receive_from(
				boost::asio::buffer(data_, max_length), _SenderEndpoint,
				boost::bind(&Osc::handle_receive_from, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_send_to(const boost::system::error_code& /*error*/,
		size_t /*bytes_sent*/)
	{
		_Socket.async_receive_from(
			boost::asio::buffer(data_, max_length), _SenderEndpoint,
			boost::bind(&Osc::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	int go(int argc, char* argv[])
	{
		try
		{
			Osc s(_IoService, _ComPort);
			_IoService.run();
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}

		return 0;
	}

};



#endif