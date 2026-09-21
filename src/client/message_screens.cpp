#ifndef MESSAGE_SCREENS_HPP
#define MESSAGE_SCREENS_HPP

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include <functional>
#include <cstdlib>
#include <string>
#include <string_view>

namespace screens {
  
  using namespace ftxui;
  using tcp = asio::ip::tcp;

  /* >=====> ErrorMessage <=====< */
  class ErrorMessage : public ComponentBase{

    std::shared_ptr<std::string> ERROR;
    
    std::shared_ptr<asio::io_context> io;
    std::shared_ptr<tcp::socket> sock;

    std::shared_ptr<ScreenInteractive> screen;

    std::shared_ptr<std::function<void()>> handler;
    ButtonOption options;
    Component button;
    Component container;

    std::string pick_cancel_glyph(void);

    public:
      ErrorMessage(std::shared_ptr<asio::io_context>, std::shared_ptr<tcp::socket>, std::shared_ptr<ScreenInteractive>, std::shared_ptr<std::string>, std::shared_ptr<std::function<void()>>);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  ErrorMessage::ErrorMessage(std::shared_ptr<asio::io_context> io_, std::shared_ptr<tcp::socket> sock_, std::shared_ptr<ScreenInteractive> screen_, std::shared_ptr<std::string> error_, std::shared_ptr<std::function<void()>> handler_) : io(io_), sock(sock_), screen(screen_), ERROR(error_), handler(handler_) {
    options = ButtonOption::Simple();
    
    options.transform = [](const EntryState& state){
      Element ele = paragraphAlignCenter(state.label);

      if(state.focused){
        ele |= color(Color::Cyan);
      }

      return ele;
    };

    button = Button(this->pick_cancel_glyph(), [&]{ (*this->handler)(); }, options);
    Add(button);
  }

  Element ErrorMessage::OnRender(){
    return vbox({
      hbox({ 
        text("     [!] ERROR MESSAGE ") | center | color(Color::Yellow) | flex,
        separatorDouble(),
        button->Render() | size(WIDTH, EQUAL, 3)
      }),
      separatorDouble(),
      paragraphAlignCenter(this->ERROR->data()) | color(Color::White) | borderEmpty,
    }) | size(WIDTH, GREATER_THAN, 40) | borderDouble | color(Color::Red) | center;
  }

  bool ErrorMessage::OnEvent(Event e){

    if(e == Event::Escape || e == Event::Character('Q') || e == Event::Character('q')){
      sock->shutdown(tcp::socket::shutdown_both);
      sock->close();
      io->stop();
      screen->Exit();
      return true;
    }

    if(e == Event::Return){
      (*handler)();
      return true;
    }

    return button->OnEvent(e);
  }

  bool ErrorMessage::Focusable() const { return true; }

  std::string ErrorMessage::pick_cancel_glyph(){
    
    if (const char* env = std::getenv("CANCEL_GLYPH")) {
        return env;
    }

    // 2. Respect NO_COLOR / dumb terminals — no fancy Unicode
    if (const char* term = std::getenv("TERM")) {
        if (std::string_view(term) == "dumb") {
            return "x";
        }
    }

    // 3. Check locale for UTF-8
    if (const char* lang = std::getenv("LC_ALL")) {
        if (std::string_view(lang).find("UTF-8") != std::string_view::npos ||
            std::string_view(lang).find("utf8")  != std::string_view::npos) {
            return "\u2715";   // ✕
        }
    }
    if (const char* lang = std::getenv("LC_CTYPE")) {
        if (std::string_view(lang).find("UTF-8") != std::string_view::npos ||
            std::string_view(lang).find("utf8")  != std::string_view::npos) {
            return "\u2715";
        }
    }
    if (const char* lang = std::getenv("LANG")) {
        if (std::string_view(lang).find("UTF-8") != std::string_view::npos ||
            std::string_view(lang).find("utf8")  != std::string_view::npos) {
            return "\u2715";
        }
    }

    // 4. Fallback: ASCII
    return "x";
  }

  /* >=====> PassPhrase Prompt <====< */
  
  class PassphrasePrompt : public ComponentBase {

    std::shared_ptr<asio::io_context> io;
    std::shared_ptr<tcp::socket> sock;
    std::shared_ptr<ScreenInteractive> screen;

    std::shared_ptr<std::string> passphrase;
    std::function<void()> onSubmit;

    Component input_field;
    Component submit_button;
    Component container;

    ButtonOption button_options;
    InputOption input_options;

    public:
      PassphrasePrompt(std::shared_ptr<asio::io_context>, std::shared_ptr<tcp::socket>, std::shared_ptr<ScreenInteractive>, std::shared_ptr<std::string>, std::function<void()>);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  PassphrasePrompt::PassphrasePrompt(std::shared_ptr<asio::io_context> io_, std::shared_ptr<tcp::socket>sock_, std::shared_ptr<ScreenInteractive> screen_, std::shared_ptr<std::string> passphrase_out_, std::function<void()> onSubmit_) : io(io_), sock(sock_), screen(screen_), passphrase(passphrase_out_), onSubmit(onSubmit_) {

      // Configure Password Input options (Masking with *)
      input_options.password = true;
      input_options.multiline = false;

      input_options.transform = [](const InputState& state){
        Element ele = state.element | center;
        if(state.focused){ return ele; }
        return ele;
      };

      input_field = Input(passphrase.get(), "Enter Passphrase", input_options);

      // Custom Simple Button Styling matching your design
      button_options = ButtonOption::Simple();
      button_options.transform = [](const EntryState& state) {
        Element ele = paragraphAlignCenter(state.label);
        if (state.focused) {
          ele |= color(Color::Cyan);
        }
        return ele;
      };

      submit_button = Button("SUBMIT", this->onSubmit, button_options);

      // Combine Input and Button in a vertical container for keyboard navigation
      container = Container::Vertical({
        input_field,
        submit_button
      });

      Add(container);
  }

  Element PassphrasePrompt::OnRender() {
    return vbox({
        hbox({ text(" [*] PASSPHRASE REQUIRED ") }) | color(Color::Cyan) | center,
        separatorDouble(),
        vbox({
          text(" Enter your GPG secret key passphrase: ") | color(Color::White) | center,
          separatorEmpty(),
          input_field->Render() | size(WIDTH, EQUAL, 32) | borderLight | center
        }) | borderEmpty,
        separatorDouble(),
        hbox({
          separatorDouble(),
          submit_button->Render() | size(WIDTH, EQUAL, 10),
          separatorDouble()
        }) | center
      }) | size(WIDTH, GREATER_THAN, 46) | borderDouble | color(Color::Blue) | center;
  }

  bool PassphrasePrompt::OnEvent(Event e) {
    if (e == Event::Escape) {
      sock->shutdown(tcp::socket::shutdown_both);
      sock->close();
      io->stop();
      screen->Exit();
      return true;
    }

    if(e == Event::Return){
      this->onSubmit();
      return true;
    }

    return container->OnEvent(e);
  }

   bool PassphrasePrompt::Focusable() const { return true; }

  /* >=====> The Error Chat Message Component <=====< */

  class ChatMessageError : public ComponentBase {
    std::string DETAILS;
    public:
      ChatMessageError(std::string);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  ChatMessageError::ChatMessageError(std::string details) : DETAILS(details) {}

  Element ChatMessageError::OnRender(){
    return vbox({
      hbox({
        text(" -- [!] Chat Message Error -- ") | center | color(Color::Yellow),
      }) | center,
      separatorHeavy(),
      paragraphAlignCenter(this->DETAILS) | color(Color::Red)
    }) | borderHeavy | color(Color::Magenta);
  }

  bool ChatMessageError::OnEvent(Event e) {
    return false;
  }

  bool ChatMessageError::Focusable() const { return true; }

  /* >=====> The Connection Message Component <=====< */
  class ConnectionMessage : public ComponentBase {

    std::shared_ptr<ScreenInteractive> screen;

    public:
      ConnectionMessage(std::shared_ptr<ScreenInteractive>);
      Element OnRender(void) override;
      bool OnEvent(Event) override;
      bool Focusable(void) const final;
  };

  ConnectionMessage::ConnectionMessage(std::shared_ptr<ScreenInteractive> screen_) : screen(screen_) {}

  Element ConnectionMessage::OnRender(){
    return vbox({
      text(" ── [*] Connection Status ── ") | color(Color::Green) | center,
      separatorHeavy(),
      paragraphAlignCenter(" Connecting to the server... ") | color(Color::White) | dim | borderEmpty
    }) | size(WIDTH, GREATER_THAN, 30) | borderHeavy | color(Color::Cyan) | center | borderDouble;
  }

  bool ConnectionMessage::OnEvent(Event e){
    if(e == Event::Escape || e == Event::Character('q') || e == Event::Character('Q') ){
      screen->Exit();
      return true;
    }
    return false;
  }

  bool ConnectionMessage::Focusable() const { return true; }

}

#endif // !MESSAGE_SCREENS_HPP

