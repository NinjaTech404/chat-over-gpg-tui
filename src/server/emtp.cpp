#ifndef EMTP_HPP
#define EMTP_HPP


#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/system.hpp>


#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <cstring>
#include <cstddef>
#include <memory>


namespace emtp{
  using tcp = boost::asio::ip::tcp;
  using boost::asio::io_context;
  using namespace boost::asio;
  
  void server(io_context* io, tcp::socket* socket,std::string ip_address, std::uint16_t port, auto handller = []{}){
    
    auto work = boost::asio::make_work_guard(*io);

    tcp::acceptor acceptor = {
      *io,
      tcp::endpoint(ip::make_address(ip_address.c_str()), port)
    };

    acceptor.async_accept(
      *socket, 
      [handller, socket, io](boost::system::error_code ec){
        if(!ec && socket->is_open()){
        boost::asio::post(*io, handller);
        }
        else{
          std::cout << ec.message() << std::endl;
        }
      }
    );
    
    io->run();
  }

  void client(std::shared_ptr<io_context> io, std::shared_ptr<tcp::socket> socket, std::string ip_address, std::uint16_t port, auto handller = []{}){

    socket->async_connect(
      tcp::endpoint(ip::make_address(ip_address), port),
      [handller, socket, io](boost::system::error_code ec){

        if(!ec && socket->is_open()){
          boost::asio::post(*io, handller);
        }
        else {
          std::cout << ec.message() << std::endl;
        }
      }
    );
  }

  std::thread receive(std::string& message, tcp::endpoint endpoint){
    return std::thread([&message, endpoint]{
      std::shared_ptr<io_context> io = std::make_shared<io_context>();
      std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(*io);
      tcp::acceptor acceptor(*io, endpoint);
      acceptor.async_accept(*socket, [&message,socket](boost::system::error_code ec){
        if(!ec){
        boost::asio::async_read(
              *socket,
              boost::asio::buffer(message),
              [socket](boost::system::error_code ec, size_t len){
                if (!ec) {
                  std::cout << "message sent successfully\n";
                }
                if (boost::asio::error::eof == ec) {
                  socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                  socket->close();
                  std::cout << "connection closed successfully\n";
                  return;
                }
                else std::cout << ec.message() << std::endl;
              });
        }
        else std::cout << "Not connected\n";
      });
      io->run();
    });
  }

  std::thread send(std::string message, boost::asio::ip::tcp::endpoint endpoint){
    return std::thread([message,endpoint]{

        std::shared_ptr<boost::asio::io_context> io = std::make_shared<boost::asio::io_context>();
        std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(*io);

        socket->async_connect(endpoint,[message, io, socket](boost::system::error_code ec){
            if (!ec) {
              boost::asio::async_write(
                  *socket,
                  boost::asio::buffer(message.data(), message.size()), 
                  [socket](boost::system::error_code ec, size_t len){
                    if(!ec && len > 0){
                      std::cout << "data sent successfully\n";
                      socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                      socket->close();
                      return;
                    }
                    else std::cout << "Faild to send data\n";
                  });
            } else std::cout << "destination is not reachable\n";
        });
        io->run();
    });
  }
}
#endif

