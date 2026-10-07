#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

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

#include <server/server_config.cpp>
#include <server/server_screens.cpp>

using namespace ftxui;

std::shared_ptr<std::chrono::steady_clock::time_point> start_time = std::make_shared<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

std::shared_ptr<asio::io_context> io = std::make_shared<asio::io_context>();

std::shared_ptr<ftxui::ScreenInteractive> server_interface = std::make_shared<ftxui::ScreenInteractive>(ScreenInteractive::FitComponent());

auto sockets = std::make_shared<std::vector<std::shared_ptr<tcp::socket>>>();

int main(int argc, char** argv){
  bool isError = false;
  auto error_message_content = std::make_shared<std::string>("NO ERROR DETECTED");
  Component server_error_message = std::make_shared<screens::ServerErrorMessage>(error_message_content);

  try{
    if(argc != 4){
      throw std::runtime_error(" [!] Invalid Arguments \n Usage: server <ip> <port> <server name> ");
    }
  }
  catch(const std::runtime_error& err){
    *error_message_content = err.what();
    isError = true;
  }

  auto work = asio::make_work_guard(*io);

  try {

    tcp::endpoint ep ( asio::ip::make_address(argv[1]), static_cast<std::uint16_t>(std::stoi(argv[2])) );

    auto acceptor = std::make_shared<tcp::acceptor>(*io, ep);

    server::acceptor(io, acceptor, sockets);

  }
  catch(const std::system_error& err){
    std::string message = "\n Usage: server <ip> <port> <server name> ";
    *error_message_content = err.what() + message;
    isError = true;
  }
  catch(const std::out_of_range& err){
    std::string message = " [!] Port Number must be 1-65535 ";
    *error_message_content = message;
    isError = true;
  }

  std::thread run_server([&]{
    io->run();
  });
  
  std::thread render_interface([&]{
    while (!isError) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      server_interface->PostEvent(Event::Custom);
    }
  });

  auto server_ip   = std::make_shared<std::string>(argv[1] ? argv [1] : "Null");
  auto server_port = std::make_shared<std::string>(argv[2] ? argv [2] : "Null");
  auto server_name = std::make_shared<std::string>(argv[3] ? argv [3] : "Null");

  std::shared_ptr<screens::ServerDashBoard> server_dashboard = std::make_shared<screens::ServerDashBoard>(
    server_name,
    server_ip,
    server_port,
    sockets,
    start_time
  );

  if(!isError){
    server_interface->Loop(server_dashboard);
  }

  else {
    work.reset();
    io->stop();
    Element error_message = server_error_message->Render();
    ftxui::Screen static_screen = ftxui::Screen::Create(ftxui::Dimension::Fit(error_message));
    ftxui::Render(static_screen, error_message);
    static_screen.Print();
  }

  
  run_server.join();
  render_interface.join();

  return 0;
}
