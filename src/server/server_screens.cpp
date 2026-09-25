#ifndef SERVER_SCREENS_HPP
#define SERVER_SCREENS_HPP

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>

#include <asio/ip/tcp.hpp>

#include <string>
#include <memory>
#include <vector>

#include <server/resources_config.cpp>

namespace screens{
  using namespace ftxui;
  /* >=====> SERVER MAIN DASHBOARD <=====< */
  class ServerDashBoard : public ComponentBase {

    // Details
    std::shared_ptr<std::string> SERVER_NAME;
    std::shared_ptr<std::string> SERVER_IP;
    std::shared_ptr<std::string> SERVER_PORT;
    std::shared_ptr<std::vector<std::shared_ptr<asio::ip::tcp::socket>>> CLIENTS;

    // Cyberpunk neon palette
    Color NEON_CYAN    = Color::RGB(0, 255, 255);   // Electric cyan
    Color NEON_MAGENTA = Color::RGB(255, 0, 255);   // Hot magenta
    Color NEON_YELLOW  = Color::RGB(255, 230, 0);   // Acid yellow
    Color NEON_GREEN   = Color::RGB(0, 255, 130);   // Matrix green
    Color NEON_RED     = Color::RGB(255, 40, 80);   // Alert red
    Color NEON_PURPLE  = Color::RGB(180, 0, 255);   // Deep purple
    Color DIM_GREY     = Color::RGB(120, 120, 140); // Muted text

    public:

      ServerDashBoard(
        std::shared_ptr<std::string>,
        std::shared_ptr<std::string>,
        std::shared_ptr<std::string>,
        std::shared_ptr<std::vector<std::shared_ptr<asio::ip::tcp::socket>>>
      );

      Element OnRender(void) override;
      bool OnEvent (Event) override;
      bool Focusable (void) const final;
  };

  ServerDashBoard::ServerDashBoard(
    std::shared_ptr<std::string> server_name,
    std::shared_ptr<std::string> server_ip,
    std::shared_ptr<std::string> server_port,
    std::shared_ptr<std::vector<std::shared_ptr<asio::ip::tcp::socket>>> clients
  ) :
    SERVER_NAME(server_name),
    SERVER_IP(server_ip),
    SERVER_PORT(server_port),
    CLIENTS(clients)
  {}

  Element ServerDashBoard::OnRender(){
    return hbox({
      // ▓▒░ NODE IDENTITY ░▒▓
      vbox({
        text("▓▒░ NODE ░▒▓") | bold | color(NEON_MAGENTA),
        separatorDouble(),
        text(" [SRV] ") | color(DIM_GREY),
        text(' ' + std::string(*this->SERVER_NAME) + ' ') | bold | color(NEON_CYAN),
        separatorDouble(),
        text(" [IP ] ") | color(DIM_GREY),
        text(' ' + std::string(*this->SERVER_IP) + ' ') | color(NEON_GREEN),
        separatorDouble(),
        text(" [PRT] ") | color(DIM_GREY),
        text(' ' + std::string(*this->SERVER_PORT) + ' ') | color(NEON_YELLOW)
      }) | borderDouble | color(NEON_MAGENTA),

      // ▓▒░ NETWORK STATUS ░▒▓
      vbox({
        text("▓▒░ LINK ░▒▓") | bold | color(NEON_CYAN),
        separatorDouble(),
        text(" [USR] ") | color(DIM_GREY),
        text(' ' + std::string(std::to_string(this->CLIENTS->size())) + ' ') | bold | color(NEON_PURPLE),
        separatorDouble(),
        text(" [SEC] ") | color(DIM_GREY),
        text(" ░OFF░ ") | bold | color(NEON_RED),
        separatorDouble(),
        text(" [STS] ") | color(DIM_GREY),
        this->CLIENTS->size() > 0 ? text(" ◉ ONLINE ") | bold | color(NEON_GREEN) : text(" ◉ OFFLINE ") | bold | color(NEON_RED)
      }) | borderDouble | color(NEON_CYAN),

      // ▓▒░ CORE METRICS ░▒▓
      vbox({
        text("▓▒░ CORE ░▒▓") | bold | color(NEON_YELLOW),
        separatorDouble(),
        text(" [CPU] ") | color(DIM_GREY),
        text(' ' + server::currentCpuUsage() + ' ') | bold | color(NEON_YELLOW),
        separatorDouble(),
        text(" [MEM] ") | color(DIM_GREY),
        text(' ' + server::currentMemoryBytes() + ' ') | bold | color(NEON_CYAN),
        separatorDouble(),
        text(" [PID] ") | color(DIM_GREY),
        text(' ' + server::currentPid() + ' ') | bold | color(NEON_MAGENTA)
      }) | borderDouble | color(NEON_YELLOW),

      // ▓▒░ RUNTIME ░▒▓
      vbox({
        text("▓▒░     PROCESS     ░▒▓") | bold | color(NEON_GREEN),
        separatorDouble(),
        text(" [UPT] ") | color(DIM_GREY),
        text(" 02:34:17 ") | color(NEON_GREEN),
        separatorDouble(),
        text(" [THR] ") | color(DIM_GREY),
        text(" 08 ") | color(NEON_GREEN),
        separatorDouble(),
        text(" >_ ") | bold | color(NEON_MAGENTA),
        text(" client " + std::string(*this->SERVER_IP) + std::string(*this->SERVER_PORT) + ' ') | color(NEON_CYAN) | bold
      }) | borderDouble | color(NEON_GREEN)

    }) | borderEmpty;
  }

  bool ServerDashBoard::OnEvent(Event e){
    return false;
  }

  bool ServerDashBoard::Focusable() const { return true; }

}

#endif
