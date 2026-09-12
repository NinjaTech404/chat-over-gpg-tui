#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>

#include <memory>
#include <string>
#include <cstddef>
#include <vector>
#include <functional>
#include <algorithm>

using tcp = asio::ip::tcp;

namespace server{

  void read_loop(const std::shared_ptr<asio::io_context>& io, const std::shared_ptr<tcp::socket>& sock, const std::shared_ptr<std::vector<std::shared_ptr<tcp::socket>>>& sockets, std::function<void(std::string)> handler){

    std::shared_ptr<asio::streambuf> buf = std::make_shared<asio::streambuf>();

    asio::async_read_until(*sock, *buf, '\0', [io, sock, sockets, handler, buf](asio::error_code ec, std::size_t bytes){
    
      std::string data;

      if(!ec && bytes > 0){
        std::istream is(buf.get());
        std::getline(is, data, '\0');
        data.push_back('\0');
        handler(data);
      }

      if(ec || ec == asio::error::eof){
        sock->shutdown(tcp::socket::shutdown_both);
        sock->close();
        auto it = std::find(sockets->begin(), sockets->end(), sock);
        if(it != sockets->end()) sockets->erase(it);
        return;
      }

      if(sock->is_open()){
        asio::post(*io, [io, sock, sockets, handler]{ read_loop(io, sock, sockets, handler); });
      }
      

    });

  }


  void acceptor (const std::shared_ptr<asio::io_context>& io, const std::shared_ptr<tcp::acceptor>& xpt, const std::shared_ptr<std::vector<std::shared_ptr<tcp::socket>>>& sockets){

    std::shared_ptr<tcp::socket> sock = std::make_shared<tcp::socket>(*io);

    xpt->async_accept(*sock, [sock, io, xpt, sockets](asio::error_code ec){

      if(!ec){
        sockets->push_back(sock);
        read_loop(io, sock, sockets, [sock, sockets](std::string data){
          for(auto& socket : *sockets) {
            if (sock.get() != socket.get()) asio::async_write(*socket, asio::buffer(data), [](auto, auto){});
          }
        });
        asio::post(*io, [io, xpt, sockets]{ acceptor(io, xpt, sockets); });
      }

    });
  }

}

#endif // !SERVER_CONFIG_HPP
