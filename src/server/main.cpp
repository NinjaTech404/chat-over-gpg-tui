
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

#include <client/gpg_config.cpp>
#include <client/client_config.cpp>
#include <server/server_config.cpp>
using namespace ftxui;

std::shared_ptr<asio::io_context> io = std::make_shared<asio::io_context>();

auto sockets = std::make_shared<std::vector<std::shared_ptr<tcp::socket>>>();

int main(int argc, char** argv){

  auto work = asio::make_work_guard(*io);

  tcp::endpoint ep ( asio::ip::make_address(argv[1]), std::stoi(argv[2]) );

  auto acceptor = std::make_shared<tcp::acceptor>(*io, ep);

  server::acceptor(io, acceptor, sockets);

  io->run();

  return 0;
}
