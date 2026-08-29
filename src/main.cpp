#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <iostream>
#include <string>


void udp_sender(){
  boost::asio::io_context io;
  boost::asio::ip::udp::socket socket(
    io,
    boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),0)
  );
  while(true){
    std::string input_sender;
    std::cout << "Enter: "; std::getline(std::cin,input_sender);
    socket.send_to(
      boost::asio::buffer(input_sender),
      boost::asio::ip::udp::endpoint(boost::asio::ip::make_address("127.0.0.1"),12345)
    );
  }
}

void udp_receiver(){
  boost::asio::io_context io;
  boost::asio::ip::udp::socket socket(
    io,
    boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),12345)
  );
  while(true){
    char msg[1024];
    boost::asio::ip::udp::endpoint sender;
    socket.receive_from(boost::asio::buffer(msg), sender);
    std::cout << "Message: " << msg << std::endl;
  }
}
int main(){
  udp_receiver();
}
