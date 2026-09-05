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

#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include <cstddef>

namespace screens {
  /* >=====> Customized Button <=====< */

  using namespace ftxui;

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
    public:
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  Element Message::OnRender(){
    return vbox({
      hbox({
        text(" client name ") | color(Color::Cyan),
        separatorHeavy(),
        text(" 42EF08B93C21423CCBAE73DABE4A8FC973D91DD0 ") | color(Color::Blue),
        separatorHeavy(),
        filler(),
        separatorHeavy(),
        text(" 2026-09-04 05:43 ") | color(Color::Yellow),
        separatorHeavy(),
        text(" Delivered ") | color(Color::Cyan)
      }),
      separatorHeavy(),
      text(" Message Body: \n - One \n - Two \n - Three ") | color(Color::White)
    }) | borderHeavy | color(Color::Green);
  }

  bool Message::OnEvent(Event e) {
    return false;
  }

  bool Message::Focusable() const { return true; }

  /* >=====> Select Client Chat Screen UI <=====< */

  class clientChatScreen : public ComponentBase{

    std::string INPUT_TEXT;
    std::shared_ptr<ScreenInteractive> screen;

    std::shared_ptr<std::string> buttonLabel = std::make_shared<std::string>(" Menu ");
    std::shared_ptr<bool> toggleMenu = std::make_shared<bool>(true);
    std::shared_ptr<customButton> menuButton = std::make_shared<customButton>(toggleMenu, buttonLabel);

    std::shared_ptr<Message> message = std::make_shared<Message>();

    InputOption option;
    Component input_;
    Component inputWrapper;
    Component container_;

    public:
      clientChatScreen(std::shared_ptr<ScreenInteractive>);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  clientChatScreen::clientChatScreen(std::shared_ptr<ScreenInteractive> screen_) : screen(screen_) {


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
      return false;
    });
    container_ = Container::Vertical({inputWrapper, menuButton, message});

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

    Element chatMessages = vbox({
      // filler(),
      // defaultChatMessage,
      filler(),
      message->Render()
    }) | flex;

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
      screen->Exit();
      return true;
    }
    return container_->OnEvent(e);
  }

  bool clientChatScreen::Focusable() const { return true; }

}

#endif
