#ifndef CLIENT_CONFIG_HPP
#define CLIENT_CONFIG_HPP

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/streambuf.hpp>
#include <asio/write.hpp>

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <cstddef>
#include <cstdint>
#include <array>
#include <istream>
#include <streambuf>
#include <functional>
#include <algorithm>
#include <vector>

#include <client/gpg_config.cpp>

using tcp = asio::ip::tcp;
using namespace nlohmann;

namespace client {


  uint32_t decode_u32 (const unsigned char* p){
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
  }

  void encode_u32 (uint32_t h, unsigned char * p){
    p[0] = (h >> 24) & 0xFF;
    p[1] = (h >> 16) & 0xFF;
    p[2] = (h >> 8) & 0xFF;
    p[3] = (h) & 0xFF;
  }

  std::string send_data (const std::shared_ptr<std::vector<GpgME::Key>>& keys, const json& j){
    std::string data = j.dump();
    std::vector<GpgME::Key> keys_ = *keys;
    std::string enc = gpg::encrypt(keys_, data);
    return enc;
  }
    
  json receive_data (std::string data, const std::shared_ptr<GpgME::Key>& key, const std::shared_ptr<std::string>& passphrase){
    
    if(!data.empty()){
      std::string decrypted = gpg::decrypt(data, *key, *passphrase);
      return json::parse(decrypted);
    }
    return json::parse("{}");
  }

  void read_loop(const std::shared_ptr<asio::io_context>& io, const std::shared_ptr<tcp::socket>& sock, std::function<void(std::string)> handler){

    std::shared_ptr<std::array<unsigned char, 4>> len_buf = std::make_shared<std::array<unsigned char, 4>>();

    asio::async_read(*sock, asio::buffer(*len_buf), [io, sock, handler, len_buf](asio::error_code ec, std::size_t){

      if(ec){
        asio::error_code ignored;
        sock->shutdown(tcp::socket::shutdown_both, ignored);
        sock->close(ignored);
        io->stop();
        return;
      }

      uint32_t n = decode_u32(len_buf->data());

      constexpr uint32_t MAX_MSG = 64u * 1024u * 1024u;

      if(n == 0 || n > MAX_MSG){
        asio::error_code ignored;
        sock->shutdown(tcp::socket::shutdown_both, ignored);
        sock->close(ignored);
        io->stop();
        return;
      }

      std::shared_ptr<std::string> data = std::make_shared<std::string>(n, '\0');

      asio::async_read(*sock, asio::buffer(*data), [io, sock, handler, data](asio::error_code ec, std::size_t){

        if(ec){
          asio::error_code ignored;
          sock->shutdown(tcp::socket::shutdown_both, ignored);
          sock->close(ignored);
          io->stop();
          return;
        }

        handler(*data);

        if(sock->is_open()){
          asio::post(*io, [io, sock, handler]{ read_loop(io, sock, handler); });
        }

      });

    });

  }
  
  void connect (const std::shared_ptr<asio::io_context>& io, const std::shared_ptr<tcp::socket>& sock, const tcp::endpoint& ep, std::function<void()> handler){
    sock->async_connect(ep, [&](asio::error_code ec){
      if(!ec){
        handler();
      }
    });
  }

  void write (const std::shared_ptr<tcp::socket>& sock, std::string data, std::function<void(asio::error_code)> handler){

    constexpr uint32_t MAX_MSG = 64u * 1024u * 1024u;
     
    if (data.empty() || data.size() > MAX_MSG){
      throw std::runtime_error(" [!] MESSAGE SIZE ERROR \n The message either has exceeded the limit size (64MB) or it is empty.");
    }

    std::shared_ptr<std::string> framed = std::make_shared<std::string>();
    framed->resize(4 + data.size());

    encode_u32(static_cast<uint32_t>(data.size()), std::bit_cast<unsigned char*>(&(*framed)[0]));

    std::copy(data.begin(), data.end(), framed->begin() + 4);

    asio::async_write(*sock, asio::buffer(*framed), [framed, handler](asio::error_code ec, std::size_t){
      if(handler) handler(ec);
    });

  }

}


#endif
