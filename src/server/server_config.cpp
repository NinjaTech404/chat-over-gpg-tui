#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>

#include <memory>
#include <string>
#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>
#include <functional>
#include <algorithm>

using tcp = asio::ip::tcp;

namespace server{

  // --- big-endian helpers (file-local) ---
  inline uint32_t decode_u32_be(const unsigned char* p) {
    return (uint32_t(p[0]) << 24) |
           (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) <<  8) |
           (uint32_t(p[3])      );
  }

  inline void encode_u32_be(uint32_t v, unsigned char* p) {
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >>  8) & 0xFF;
    p[3] = (v      ) & 0xFF;
  }

  void read_loop(const std::shared_ptr<asio::io_context>& io, const std::shared_ptr<tcp::socket>& sock, const std::shared_ptr<std::vector<std::shared_ptr<tcp::socket>>>& sockets, std::function<void(std::shared_ptr<std::string>)> handler){

    std::shared_ptr<std::array<unsigned char, 4>> len_buf = std::make_shared<std::array<unsigned char, 4>>();

    asio::async_read(*sock, asio::buffer(*len_buf), [io, sock, sockets, handler, len_buf](asio::error_code ec, std::size_t){

      if(ec){
        asio::error_code ignored;
        sock->shutdown(tcp::socket::shutdown_both, ignored);
        sock->close(ignored);
        auto it = std::find(sockets->begin(), sockets->end(), sock);
        if(it != sockets->end()) sockets->erase(it);
        return;
      }

      uint32_t n = decode_u32_be(len_buf->data());

      constexpr uint32_t MAX_MSG = 64u * 1024u * 1024u;
      if(n == 0 || n > MAX_MSG){
        asio::error_code ignored;
        sock->shutdown(tcp::socket::shutdown_both, ignored);
        sock->close(ignored);
        auto it = std::find(sockets->begin(), sockets->end(), sock);
        if(it != sockets->end()) sockets->erase(it);
        return;
      }

      std::shared_ptr<std::string> data = std::make_shared<std::string>(n, '\0');

      asio::async_read(*sock, asio::buffer(*data), [io, sock, sockets, handler, data](asio::error_code ec, std::size_t){

        if(ec){
          asio::error_code ignored;
          sock->shutdown(tcp::socket::shutdown_both, ignored);
          sock->close(ignored);
          auto it = std::find(sockets->begin(), sockets->end(), sock);
          if(it != sockets->end()) sockets->erase(it);
          return;
        }

        handler(data);

        if(sock->is_open()){
          asio::post(*io, [io, sock, sockets, handler]{ read_loop(io, sock, sockets, handler); });
        }

      });

    });

  }


  void acceptor (const std::shared_ptr<asio::io_context>& io, const std::shared_ptr<tcp::acceptor>& xpt, const std::shared_ptr<std::vector<std::shared_ptr<tcp::socket>>>& sockets){

    std::shared_ptr<tcp::socket> sock = std::make_shared<tcp::socket>(*io);

    xpt->async_accept(*sock, [sock, io, xpt, sockets](asio::error_code ec){

      if(!ec){
        sockets->push_back(sock);
        read_loop(io, sock, sockets, [sock, sockets](std::shared_ptr<std::string> data){

          // Re-frame as [4-byte BE length][payload] so clients can read it back
          // with the same length-prefixed protocol.
          std::shared_ptr<std::string> framed = std::make_shared<std::string>();
          framed->resize(4 + data->size());
          encode_u32_be(static_cast<uint32_t>(data->size()),
                        reinterpret_cast<unsigned char*>(&(*framed)[0]));
          std::copy(data->begin(), data->end(), framed->begin() + 4);

          for(auto& socket : *sockets) {
            if (sock.get() != socket.get() && socket->is_open())
              asio::async_write(*socket, asio::buffer(*framed), [framed](auto, auto){});
          }
        });
        asio::post(*io, [io, xpt, sockets]{ acceptor(io, xpt, sockets); });
      }

    });
  }

}

#endif
