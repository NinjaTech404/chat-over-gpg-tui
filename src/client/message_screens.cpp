#ifndef MESSAGE_SCREENS_HPP
#define MESSAGE_SCREENS_HPP

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>

namespace screens {
  
  using namespace ftxui;

  /* >=====> ErrorMessage <=====< */
  class ErrorMessage : public ComponentBase{

    std::shared_ptr<std::string> ERROR;
    std::shared_ptr<ScreenInteractive> screen;
    std::shared_ptr<std::function<void()>> handler;
    ButtonOption options;
    Component button;
    Component container;

    public:
      ErrorMessage(std::shared_ptr<ScreenInteractive> screen, std::shared_ptr<std::string>, std::shared_ptr<std::function<void()>>);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  ErrorMessage::ErrorMessage(std::shared_ptr<ScreenInteractive> screen_, std::shared_ptr<std::string> error_, std::shared_ptr<std::function<void()>> handler_) : screen(screen_), ERROR(error_), handler(handler_) {
    options = ButtonOption::Simple();
    
    options.transform = [](const EntryState& state){
      Element ele = paragraphAlignCenter(state.label);

      if(state.focused){
        ele |= color(Color::Cyan);
      }

      return ele;
    };

    button = Button("OK", [&]{ (*this->handler)(); }, options);
    Add(button);
  }
  Element ErrorMessage::OnRender(){
    return vbox({
      hbox({ text(" [!] ERROR MESSAGE ") }) | color(Color::Yellow) | center,
      separatorDouble(),
      paragraphAlignCenter(this->ERROR->data()) | color(Color::White) | borderEmpty,
      separatorDouble(),
      hbox({ 
        separatorDouble(),
        button->Render() | size(WIDTH, EQUAL, 6),
        separatorDouble()
      }) | center
    }) | size(WIDTH, GREATER_THAN, 40) | borderDouble | color(Color::Red) | center;
  }

  bool ErrorMessage::OnEvent(Event e){

    if(e == Event::Escape || e == Event::Character('Q') || e == Event::Character('q')){
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


  /* >=====> PassPhrase Prompt <====< */
  
  class PassphrasePrompt : public ComponentBase {
    std::shared_ptr<ScreenInteractive> screen;
    std::shared_ptr<std::string> passphrase;
    std::function<void()> onSubmit;

    Component input_field;
    Component submit_button;
    Component container;

    ButtonOption button_options;
    InputOption input_options;

    public:
      PassphrasePrompt(std::shared_ptr<ScreenInteractive>, std::shared_ptr<std::string>, std::function<void()>);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  PassphrasePrompt::PassphrasePrompt(std::shared_ptr<ScreenInteractive> screen_, std::shared_ptr<std::string> passphrase_out_, std::function<void()> onSubmit_) : screen(screen_), passphrase(passphrase_out_), onSubmit(onSubmit_) {

      // Configure Password Input options (Masking with *)
      input_options.password = true;
      input_options.multiline = false;

      input_options.transform = [](const InputState& state){
        Element ele = state.element | center;
        if(state.focused){ return ele; }
        return ele;
      };

      input_field = Input(passphrase.get(), "Enter Passphrase...", input_options);

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


}

#endif // !MESSAGE_SCREENS_HPP

