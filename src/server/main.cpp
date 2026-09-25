
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

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
#include <exception>

#include <client/gpg_config.cpp>
#include <client/client_config.cpp>
#include <server/server_config.cpp>
#include <server/server_screens.cpp>


using namespace ftxui;

std::shared_ptr<asio::io_context> io = std::make_shared<asio::io_context>();

std::shared_ptr<ftxui::ScreenInteractive> server_interface = std::make_shared<ftxui::ScreenInteractive>(ScreenInteractive::FitComponent());

auto sockets = std::make_shared<std::vector<std::shared_ptr<tcp::socket>>>();

int main(int argc, char** argv){

  if(argc != 4){
    throw std::runtime_error(" [!] Invalid Arguments \n Usage: server <ip> <port> <server name>");
    return -1;
  }

  auto work = asio::make_work_guard(*io);

  tcp::endpoint ep ( asio::ip::make_address(argv[1]), std::stoi(argv[2]) );

  auto acceptor = std::make_shared<tcp::acceptor>(*io, ep);

  server::acceptor(io, acceptor, sockets);

  std::thread run_server([&]{
    io->run();
  });


  auto server_ip   = std::make_shared<std::string>(argv[1] ? argv [1] : "Null");
  auto server_port = std::make_shared<std::string>(argv[2] ? argv [2] : "Null");
  auto server_name = std::make_shared<std::string>(argv[3] ? argv [3] : "Null");

  std::shared_ptr<screens::ServerDashBoard> server_dashboard = std::make_shared<screens::ServerDashBoard>(
    server_name,
    server_ip,
    server_port,
    sockets
  );

  server_interface->Loop(server_dashboard);

  run_server.join();

  return 0;
}
