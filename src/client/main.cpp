#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <deque>
#include <vector>
#include <thread>
#include <functional>

#include <client/scroller.cpp>

#include <nlohmann/json.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <server/emtp.cpp>



using namespace ftxui;
using tcp = boost::asio::ip::tcp;
using boost::asio::io_context;
using namespace boost;


const char* description_sample = "Just a bunch of devs and sysadmins who love building stuff, breaking stuff, and learning from the chaos. We talk about cloud-native architecture, CI/CD pipelines, container orchestration, and the occasional existential crisis about microservices. All skill levels welcome – bring your curiosity and your favorite terminal emulator.";

static auto ui = ScreenInteractive::Fullscreen();
class Message {
  private:
    std::string ID;
    std::string DATE;
    std::string SENDER;
    std::string CONTENT;
    bool DELIVERED = false;
  public:
    Message(
      std::string id,
      std::string date,
      std::string sender,
      std::string content,
      bool delivered = true
    ) : ID(id),
        DATE(date),
        SENDER(sender),
        CONTENT(content),
        DELIVERED(delivered)
    {};

  ftxui::Element show(){
    return vbox({
      hbox({
        text(" " + SENDER + " ") | color(Color::Cyan),
        separatorLight(),
        text(" " + ID + " ") | color(Color::Magenta),
        separatorLight(),
        filler() | flex,
        separatorLight(),
        text(" " + DATE + " ") | color(Color::Yellow),
        separatorLight(),
        text(DELIVERED? " Delivered " : " Failed ") | color(DELIVERED ? Color::Green : Color::Red)
      }),
      separatorLight(),
      hbox({text(CONTENT) | color(Color::White)})
    }) | borderRounded | color(Color::Blue);
  }

  std::string getId(){
    return this->ID;
  }
  std::string getDate(){
    return this->DATE;
  }
  std::string getSender(){
    return this->SENDER;
  }
  std::string getContent(){
    return this->CONTENT;
  }
  bool isDelivered(){
    return this->DELIVERED;
  }
};

std::vector<Element> messages = {filler()};

auto text_input (std::string& message, std::shared_ptr<io_context> io, std::shared_ptr<tcp::socket> socket){
  auto input = Input(&message, "Type a message") | color(Color::Black) | bgcolor(Color::White);
  return CatchEvent(input, [&message, io, socket](Event e){
    if(e == Event::Return && message.size() != 0){
      
      asio::post(*io, [socket, msg = message]{
        socket->async_write_some(asio::buffer(msg), [](system::error_code ec, std::size_t len){
          ui.PostEvent(Event::Custom);
        });
      });

      messages.push_back(
        Message(
          std::string("EC7F 9A8B C3ED 0477 FDA2  D2D8 F61B F7B3 1B5B 7B6D"),
          std::string("2026-08-21"),
          std::string("KADHIM SHAKIR KADHIM"),
          message
        ).show()
      );
      message.clear();
      return true;
    }
    return false;
  });
}


auto chat_screen (){
    return Scroller(
      Renderer([&]{
          return vbox(messages);
        }
      )
  );
}




std::thread tcp_connection(std::shared_ptr<io_context> io, std::shared_ptr<tcp::socket> socket, tcp::endpoint endpoint){
  return std::thread([io, socket, endpoint]{
    auto data = std::make_shared<std::string>();
    socket->async_connect(endpoint, [io, socket, endpoint, data](system::error_code ec){
      

      asio::post(*io,[socket]{
          nlohmann::json j = nlohmann::json({
            {"name", "KADHIM SHAKIR KADHIM"},
            {"id", "EC7F 9A8B C3ED 0477 FDA2  D2D8 F61B F7B3 1B5B 7B6D"}
          });
        socket->async_write_some(asio::buffer(j.dump().data(), 512), [](system::error_code ec, std::size_t le){});
      });

      auto read_loop = std::make_shared<std::function<void()>>();
      *read_loop = [io, socket, read_loop]{

        auto data = std::make_shared<std::string>(1024,'\0');
        socket->async_read_some(
          asio::buffer(*data),
          [io, socket, data, read_loop](system::error_code ec, std::size_t len){
            
            if (!ec && len > 0) {
              data->resize(len);

              char text[len + 1] = {};
              for(int i = 0; i < len; i++){
                text[i] = (*data)[i];
              }
              text[len] = '\0';

              messages.push_back(
                Message(
                  std::string("EC7F 9A8B C3ED 0477 FDA2  D2D8 F61B F7B3 1B5B 7B6D"),
                  std::string("2026-08-21"),
                  std::string("KADHIM SHAKIR KADHIM"),
                  std::string(text)
                ).show()
              );
              data->resize(1024);

              ui.PostEvent(Event::Custom);
            }
          

            if(socket->is_open() && !ec){
              asio::post(*io,*read_loop);
            }

          }
        );
      
      };

      asio::post(*io,*read_loop);

    });
    io->run();
  });
}



auto client_screen(std::shared_ptr<io_context> io, std::shared_ptr<tcp::socket> socket){
  static std::string message;
  static auto message_input = text_input(message, io, socket);
  static auto chat = chat_screen();
  static auto container = Container::Vertical({chat, message_input});
  
  return Renderer(container, [&]{
      return vbox({
          hbox(text("The Cosmic Group") | flex, separatorDouble(), text("Menu") ),
          separatorDouble(),
          hbox({
            chat->Render() | flex,
          }) | flex,
          separatorDouble(),
          message_input->Render()
      }) | borderDouble;
  });
}



int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        return 1;
    }
  
    auto io = std::make_shared<io_context>();
    auto socket = std::make_shared<tcp::socket>(*io);
    auto endpoint = tcp::endpoint(asio::ip::make_address(argv[1]), std::stoi(argv[2]));


    auto work = make_work_guard(*io);
    auto chat_connection =  tcp_connection(io, socket, endpoint);

    ui.Loop(client_screen(io, socket));
    
    chat_connection.join();
    return 0;
}
