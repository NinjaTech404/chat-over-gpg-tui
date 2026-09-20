#ifndef CLIENT_CHAT_SCREEN
#define CLIENT_CHAT_SCREEN

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <gpgme++/context.h>
#include <gpgme++/key.h>
#include <gpgme++/engineinfo.h>
#include <gpgme++/keylistresult.h>

#include <nlohmann/json.hpp>

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <cstring>
#include <vector>
#include <memory>
#include <cstddef>
#include <initializer_list>
#include <functional>
#include <chrono>

#include <client/client_config.cpp>

enum class Screens : char { Login = 0, RecipientMenu = 1, ClientScreen = 2, PassphrasePrompt = 3, ErrorMessage = 4};

#include <client/client_config.cpp>

namespace screens {
  /* >=====> Customized Button <=====< */

  using namespace ftxui;
  using namespace nlohmann;

  class customButton : public ComponentBase{
    std::shared_ptr<std::string> LABEL;
    std::shared_ptr<bool> isClicked;
    Box box_;
    public:
      customButton(std::shared_ptr<bool>, std::shared_ptr<std::string>);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  customButton::customButton(std::shared_ptr<bool> clicked, std::shared_ptr<std::string> label) : isClicked(clicked), LABEL(label) {}

  Element customButton::OnRender() {
    return text(*(this->LABEL)) | center | reflect(box_);
  };

  bool customButton::OnEvent(Event e){
    if(e.is_mouse()){
      Mouse mouse = e.mouse();
      if(box_.Contain(mouse.x, mouse.y)){
        if(mouse.button == Mouse::Left && mouse.motion == Mouse::Pressed){
          TakeFocus();
          *isClicked = !(*isClicked);
          return true;
        }
      }
    }
    return false;
  }

  bool customButton::Focusable() const { return true; }
  
  /* >=====> The Message Component <=====< */

  class Message : public ComponentBase {
    std::string NAME;
    std::string FINGERPRINT;
    std::string DATE;
    int STATUS;
    std::string DATA;
    public:
      Message(std::string, std::string, std::string, int, std::string);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  Message::Message(std::string name, std::string fingerprint, std::string date, int status, std::string data) 
    : NAME(name), FINGERPRINT(fingerprint), DATE(date), STATUS(status), DATA(data){}

  Element Message::OnRender(){
    return vbox({
      hbox({
        text(' ' + this->NAME + ' ') | color(Color::Cyan),
        separatorHeavy(),
        text(' ' + this->FINGERPRINT + ' ') | color(Color::Blue),
        separatorHeavy(),
        filler(),
        separatorHeavy(),
        text(' ' + this->DATE + ' ') | color(Color::Yellow),
        separatorHeavy(),
        text(this->STATUS == 200? " Delivered " : " Failed ") | color(Color::Cyan)
      }),
      separatorHeavy(),
      paragraph(this->DATA) | color(Color::White)
    }) | borderHeavy | color(Color::Green);
  }

  bool Message::OnEvent(Event e) {
    return false;
  }

  bool Message::Focusable() const { return true; }

  /* >=====> Chat Screen UI <=====< */

  class chatScreen : public ComponentBase {

    Components messages;
    Component scroller;
    Component container;

    public:
      chatScreen(Components);
      void update(Components);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  chatScreen::chatScreen(Components messages_) : messages(messages_){
    container = Container::Vertical(messages);
    scroller = Scroller(container);
    Add(scroller);
  }

  void chatScreen::update(Components messages_){
    container->DetachAllChildren();

    for(auto& message : messages_) {
      container->Add(message);
    }

  }

  Element chatScreen::OnRender(){

    return vbox({
      scroller->Render()
    });
  }

  bool chatScreen::OnEvent(Event e) {
    return scroller->OnEvent(e);
  }

  bool chatScreen::Focusable() const { return true; }

  /* >=====> Client Chat Screen UI <=====< */

  class clientChatScreen : public ComponentBase{

    std::string INPUT_TEXT;
    std::shared_ptr<asio::io_context> io;
    std::shared_ptr<tcp::socket> sock;
    std::shared_ptr<ScreenInteractive> screen;

    int currentScreen;
    std::shared_ptr<std::string> error_message_content;
    std::shared_ptr<std::function<void()>> error_message_handler;


    std::shared_ptr<GpgME::Key> clientAccount;
    std::shared_ptr<std::vector<GpgME::Key>> recipients;

    
    std::shared_ptr<std::string> buttonLabel = std::make_shared<std::string>(" Menu ");
    std::shared_ptr<bool> toggleMenu = std::make_shared<bool>(true);
    std::shared_ptr<customButton> menuButton = std::make_shared<customButton>(toggleMenu, buttonLabel);

    json json_data;
    std::shared_ptr<Components> messages;
    std::shared_ptr<chatScreen> chat;
    

    InputOption option;
    Component input_;
    Component inputWrapper;
    Component container_;

    public:
      clientChatScreen(
          std::shared_ptr<asio::io_context>, 
          std::shared_ptr<tcp::socket>, 
          std::shared_ptr<ScreenInteractive>, 
          std::shared_ptr<Components>, 
          const std::shared_ptr<GpgME::Key>&, 
          const std::shared_ptr<std::vector<GpgME::Key>>&, 
          int&, 
          const std::shared_ptr<std::string>&, 
          const std::shared_ptr<std::function<void()>>&);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  clientChatScreen::clientChatScreen(
      std::shared_ptr<asio::io_context> io_,
      std::shared_ptr<tcp::socket> sock_,
      std::shared_ptr<ScreenInteractive> screen_,
      std::shared_ptr<Components> messages_,
      const std::shared_ptr<GpgME::Key>& clientAccount_,
      const std::shared_ptr<std::vector<GpgME::Key>>& recipients_,
      int& currentScreen_,
      const std::shared_ptr<std::string>& error_message_content_,
      const std::shared_ptr<std::function<void()>>& error_message_handler_ ): io(io_), sock(sock_), screen(screen_), messages(messages_), clientAccount(clientAccount_), recipients(recipients_), currentScreen(currentScreen_), error_message_content(error_message_content_), error_message_handler(error_message_handler_) {


    option.transform = [](InputState state){
      Element ele = state.element;
      if(state.is_placeholder){
        ele |= ftxui::bgcolor(ftxui::Color::Default) | ftxui::color(ftxui::Color::White) | dim;
      }
      if(state.focused){
        ele |= ftxui::bgcolor(ftxui::Color::Default) | ftxui::color(ftxui::Color::White);
      }
      return ele;
    };


    input_ = Input(&INPUT_TEXT, "Type a message", option);
    inputWrapper = CatchEvent(input_, [&](Event e){
      if(e == Event::Return){
        if(recipients->size() > 0 && !this->INPUT_TEXT.empty()){
          try{

            json_data["name"] = clientAccount->userID(0).name();
            json_data["fingerprint"] = clientAccount->primaryFingerprint();
            json_data["date"] = std::format("{:%Y-%m-%d %H:%M}", std::chrono::system_clock::now());
            json_data["status"] = 200;
            json_data["data"] = this->INPUT_TEXT;
        
            std::string encrypted = client::send_data(recipients, json_data);

            Component message = std::make_shared<Message>(
              json_data["name"].get<std::string>(),
              json_data["fingerprint"].get<std::string>(),
              json_data["date"].get<std::string>(),
              json_data["status"].get<int>(),
              json_data["data"].get<std::string>()
            );

            this->messages->push_back(message);
            
            asio::post(*io, [date = encrypted, this]{
              client::write(sock, std::move(date), [this](asio::error_code er){
                if(!er){
                  screen->PostEvent(Event::Custom);
                }
              });
            });
            this->INPUT_TEXT.clear();

          }
          catch (const std::runtime_error& err){
            *error_message_content = err.what();
            *error_message_handler = [&]{
              currentScreen = static_cast<int>(Screens::ClientScreen);
            };
            currentScreen = static_cast<int>(Screens::ErrorMessage);
          }
        }
        return true;
      }
      return false;
    });

    chat = std::make_shared<chatScreen>(*messages);

    container_ = Container::Vertical({inputWrapper, menuButton, chat});

    Add(container_);
  }

  Element clientChatScreen::OnRender(){


    Element sideBarContent = vbox({
      // ── Section 1: Online Clients ──
      vbox({
        text(" ONLINE CLIENTS ") | bold | color(Color::Cyan),
        separatorDouble(),
        hbox({ text(" ● ") | color(Color::Green), text(" Alice "), filler(), text("42EF...91DD") | dim }) | color(Color::White),
        hbox({ text(" ● ") | color(Color::Green), text(" Bob "),   filler(), text("A1B2...34C5") | dim })  | color(Color::White),
        hbox({ text(" ● ") | color(Color::GrayDark), text(" Charlie "), filler(), text("Offline") | dim })  | color(Color::White),
        filler()
      }) | flex,

      separatorDouble(),
      // ── Section 2: TUI Usage / Hotkeys ──
      vbox({
        text(" TUI USAGE ") | bold | color(Color::Yellow),
        separatorDouble(),
        hbox({ text(" • Enter ") | bold, filler(), text("Send message") | dim }) | color(Color::Cyan),
        hbox({ text(" • Tab ")   | bold, filler(), text("Focus next input") | dim }) | color(Color::Cyan),
        hbox({ text(" • Esc ")   | bold, filler(), text("Toggle sidebar") | dim }) | color(Color::Cyan),
        hbox({ text(" • ↑ / ↓ ") | bold, filler(), text("Scroll messages") | dim }) | color(Color::Cyan),
        hbox({ text(" • Ctrl+C ") | bold, filler(), text("Quit application") | dim }) | color(Color::Cyan),
        filler()
      }) | flex,

    }) | flex;


    Element sideBar = [&] {
      
      if(*toggleMenu){
        *buttonLabel = " Close ";
        return vbox({ 
            hbox({
              paragraphAlignCenter(" ── Menu Options ── ") | color(Color::White) | flex,
              *toggleMenu ? separatorDouble() : emptyElement(),
              *toggleMenu ? menuButton->Render() | color(Color::Magenta) : emptyElement(),
            }),
            separatorDouble(),
            sideBarContent
        }) | borderDouble | flex | size(WIDTH, EQUAL, 40) | color(Color::Blue);
      }
      else{
        *buttonLabel = " Menu ";
        return emptyElement();
      }
      
    }();

    Element header = hbox({
      paragraphAlignCenter(" ── Server Name ── ") | color(Color::White) | flex,
      !(*toggleMenu) ? separatorDouble() : emptyElement(),
      !(*toggleMenu) ? menuButton->Render() | color(Color::Magenta) : emptyElement(),
    }) | color(Color::Cyan);

    Element defaultChatMessage = vbox({
      hbox({ paragraphAlignCenter(" ── [!] EMPTY CHAT ── ")}) | center,
      separatorHeavy(),
      paragraphAlignCenter(" No messages yet? Wait until someone sends a message \n or you can send the first message so others can see it. ")
    }) | borderHeavy | center | color(Color::Yellow);

    Element chatMessages = [&]{
      
      if(messages->size() > 0){
        chat->update(*messages);
        return vbox({
          filler(),
          chat->Render() | color(Color::White)
        }) | flex;
      }
      else{
        return vbox({
          filler(),
          defaultChatMessage,
          filler(),
        }) | flex;
      }

    }();

    Element messageInput = vbox({
      hbox({
        text(" "),
        inputWrapper->Render()
      }) | borderDouble | color(Color::Magenta)
    });

    return hbox({
      vbox({
          vbox({
            header,
            separatorDouble(),
            chatMessages | flex,
          }) | borderDouble | color(Color::Cyan) | flex,
          messageInput,
      }) | flex,
      sideBar
    }) | flex;
  }
  
  bool clientChatScreen::OnEvent(Event e){
    if(e == Event::Escape){
      sock->shutdown(tcp::socket::shutdown_both);
      sock->close();
      io->stop();
      screen->Exit();
      if(recipients){ std::cout << recipients->size() << std::endl;}
      return true;
    }
    return container_->OnEvent(e);
  }

  bool clientChatScreen::Focusable() const { return true; }

}

#endif
