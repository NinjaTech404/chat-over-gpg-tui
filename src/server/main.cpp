#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/system.hpp>

#include <nlohmann/json.hpp>

#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <cstring>
#include <cstddef>
#include <memory>
#include <functional>
#include <vector>
#include <server/emtp.cpp>



using json = nlohmann::json;

std::vector<json> users;

using namespace boost;
using namespace std;
using tcp = boost::asio::ip::tcp;
using boost::asio::io_context;
using namespace boost;


static io_context io;
std::vector<std::shared_ptr<tcp::socket>> clients;

std::thread server (tcp::endpoint ep){
  return std::thread([ep]{

    auto work = make_work_guard(io);
    
    tcp::acceptor acceptor(io, ep);

    auto read_loop = std::make_shared<std::function<void(std::shared_ptr<tcp::socket> sock)>>();
    *read_loop = [read_loop](std::shared_ptr<tcp::socket> sock){
      auto data = std::make_shared<std::string>(1024,'\0');
      sock->async_read_some(
      asio::buffer(*data),
        [sock, data, read_loop](system::error_code ec, std::size_t len){

          if (!ec && len > 0) {
            data->resize(len);
            // std::cout << *data << std::endl;
            

            for(auto client : clients){
              if(sock != client && client->is_open()){
                std::string message = *data;
                client->async_write_some(asio::buffer(message.data(), 1024), [](system::error_code ec, std::size_t len){});
              }
            } 

            data->resize(1024);
          }

          if(sock->is_open() && !ec){
            asio::post(io, [read_loop, sock]{ (*read_loop)(sock);});
          }
          else {
            sock->shutdown(tcp::socket::shutdown_both);
            sock->close();
            auto it = std::find(clients.begin(), clients.end(), sock);
            if(it != clients.end()){
              clients.erase(it);
            }
          }

        }
      );
    };

    auto sock_loop = std::make_shared<std::function<void()>>();
    *sock_loop = [&acceptor, sock_loop, read_loop]{

      auto data = std::make_shared<std::string>(512, '\0');
      auto sock = std::make_shared<tcp::socket>(io);

      acceptor.async_accept(*sock, [sock, data, sock_loop, read_loop](system::error_code ec){

        clients.push_back(sock);

        sock->async_read_some(
          asio::buffer(*data),
          [sock, data, sock_loop](system::error_code ec, std::size_t len){

            if (!ec && len == 512) {
              data->resize(len);
              nlohmann::json j = nlohmann::json::parse(*data);
              tcp::endpoint remote_ep = sock->remote_endpoint();
              j["endpoint"] = {{"ip", remote_ep.address().to_string()}, {"port", remote_ep.port()}};
              users.push_back(j);
              // std::cout << j << std::endl;
              data->resize(512);
            }
            asio::post(io,*sock_loop);
          }

        );

        asio::post(io, [read_loop, sock]{ (*read_loop)(sock);});

      });

    };

    

    

    asio::post(io,*sock_loop);
        


    io.run();
  });
};



int main(int argc, char** argv){

  std::thread start = server(tcp::endpoint(asio::ip::make_address(argv[1]), std::stoi(argv[2])));
  
  start.join();

  return 0;
}
