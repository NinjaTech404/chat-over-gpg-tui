#ifndef CLIENT_CONFIG_HPP
#define CLIENT_CONFIG_HPP

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <asio/write.hpp>

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <cstddef>
#include <istream>
#include <streambuf>
#include <functional>
#include <algorithm>
#include <vector>

#include <client/gpg_config.cpp>

using tcp = asio::ip::tcp;
using namespace nlohmann;

namespace client {

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

    std::shared_ptr<asio::streambuf> buf = std::make_shared<asio::streambuf>();

    asio::async_read_until(*sock, *buf, '\0', [io, sock, handler, buf](asio::error_code ec, std::size_t bytes){
    
      std::string data;

      if(!ec && bytes > 0){
        std::istream is(buf.get());
        std::getline(is, data, '\0');
        //data.push_back('\0');
        handler(data);
      }

      if(ec || ec == asio::error::eof){
        sock->shutdown(tcp::socket::shutdown_both);
        sock->close();
        io->stop();
        return;
      }

      if(sock->is_open()){
        asio::post(*io, [io, sock, handler]{ read_loop(io, sock, handler); });
      }
      

    });

  }
  
  void connect (const std::shared_ptr<asio::io_context>& io, const std::shared_ptr<tcp::socket>& sock, const tcp::endpoint& ep, std::function<void()> handler){
    sock->async_connect(ep, [&](asio::error_code ec){
      if(!ec){
        handler();
      }
    });
  }
}


#endif
