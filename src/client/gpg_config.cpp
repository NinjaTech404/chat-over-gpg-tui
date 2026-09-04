#ifndef GPG_CONFIG_HPP
#define GPG_CONFIG_HPP

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

namespace gpgui{

  using namespace ftxui;

  /* >=====> Login UI <=====< */
  class login : public ComponentBase{
    
    std::string HEADER = " You might select only one GPG account of yours.\n "
                         " NOTE: make sure your PRIVATE/SECRET key is present. ";
   
    std::string FOOTER = " Use the arows up/down or the k/j buttons to select the key \n"
                         " press ENTER to confirm or ESC/Q to quit. ";

    std::vector<GpgME::Key> keys;
    
    int selected = 0;
    
    std::shared_ptr<ScreenInteractive> screen;
   
    
    public:
      login(std::shared_ptr<ScreenInteractive> screen_);
      std::shared_ptr<GpgME::Key> getSelected();
      Element OnRender() override;
      bool OnEvent(Event e) override;
      bool Focusable() const final;

  };

  login::login(std::shared_ptr<ScreenInteractive> screen_) : screen(screen_){

    GpgME::initializeLibrary();
    auto ctx = std::unique_ptr<GpgME::Context>(GpgME::Context::create(GpgME::OpenPGP));

    ctx->setKeyListMode(GpgME::Local | GpgME::KeyListMode::WithSecret);

    GpgME::Error err;

    auto result = ctx->startKeyListing("");
    
    while(true){
      GpgME::Key key = ctx->nextKey(err);

      if(key.hasSecret()){
        keys.push_back(key);
      }

      if(err || key.isNull()){
        break;
      }
    }

    ctx->endKeyListing();

  }

  std::shared_ptr<GpgME::Key> login::getSelected(){ 
    return std::make_shared<GpgME::Key>(this->keys.at(this->selected)); 
  }

  Element login::OnRender(){

    std::vector<Element> gpgAccounts;

    for(int i = 0; i < keys.size(); i++) {
      gpgAccounts.push_back(
        vbox({
          hbox({
            text(std::string(" Name: ") + keys[i].userID(0).name() + ' ') | flex,
            separatorHeavy(),
            text(std::string(" Email: ") + keys[i].userID(0).email() + ' ') | flex
          }),
          separatorHeavy(),
          text(std::string(" Fingerprint: ") + keys[i].primaryFingerprint() + ' ')
        }) | borderHeavy | (selected == i? color(Color::Cyan) : color(Color::White) | dim)
      );
    }
    
    Element defaultMessage = vbox({
      paragraphAlignCenter( " ── [!] WARNING ── " ) | color(Color::Yellow),
      separatorDouble(),
      paragraphAlignCenter(" It seems that you either don't have GnuPG installed, \n you haven't yet initialized your keyrings or you \n didn't CREATE/IMPORT your personal GnuPG keys. \n please manually check that you have one. ") | color(Color::Yellow),
      separatorDouble(),
      paragraphAlignCenter(" press ESC/Q to quit. ") | color(Color::Magenta)
    }) | borderDouble | color(Color::Blue) | center;

    Element keysMenu = vbox({
      paragraphAlignCenter(this->HEADER) | color(Color::Yellow),
      separatorDouble(),
      vbox(gpgAccounts),
      separatorDouble(),
      paragraphAlignCenter(this->FOOTER) | color(Color::Magenta)
    }) | borderDouble | color(Color::Blue) | center;

    return keys.size() > 0? keysMenu : defaultMessage;
  }

  bool login::OnEvent(Event e){
    if (e == Event::ArrowUp || e ==  Event::Character('k') || e == Event::Character('K')) {
        if (selected > 0) {
            selected--;
        } else {
            selected = keys.size() - 1;
        }
        return true;
    }

    if (e == Event::ArrowDown || e == Event::Character('j') || e == Event::Character('J')) {
        if (selected < keys.size() - 1) {
            selected++;
        } else {
            selected = 0;
        }
        return true;
    }

    if (e == Event::Escape || e == Event::Character('q') || e == Event::Character('Q')){
      screen->Exit();
      return true;
    }

    return false;
  }

  bool login::Focusable() const { return true; }

  /* >=====> Select Recipient UI <=====< */

  class recipientMenu : public ComponentBase {

    std::string HEADER = " Select the recipient (PUBLIC key) accounts to start chatting. \n"
                         " You can select multiple recipients keys. ";
    std::string FOOTER = " Use the arows (UP/DOWN) or the (K/J) buttons to navigate. \n"
                         " Press (T) to select a recipient key or (U) to unselect. \n"
                         " Press ENTER to confirm or ESC/Q to quit. ";

    std::shared_ptr<ScreenInteractive> screen;
    std::shared_ptr<GpgME::Key> clientKey;

    std::vector<GpgME::Key> keys;
    std::vector<GpgME::Key> recipients;

    int selected = 0;

    Element getTrustLevel(GpgME::Key);

    bool isRecipientKeySelected (GpgME::Key);

    public:
      recipientMenu(std::shared_ptr<ScreenInteractive>);
      Element OnRender () override;
      bool OnEvent (Event) override;
      bool Focusable() const final;
      std::shared_ptr<std::vector<GpgME::Key>> getRecipientsKeys(void) const;
  };


  recipientMenu::recipientMenu(std::shared_ptr<ScreenInteractive> screen_) : screen(screen_) {

    GpgME::initializeLibrary();

    std::unique_ptr<GpgME::Context> ctx = std::unique_ptr<GpgME::Context>(GpgME::Context::create(GpgME::OpenPGP));
    
    ctx->setKeyListMode(GpgME::Local | GpgME::KeyListMode::WithSecret | GpgME::KeyListMode::Validate);
    
    auto result = ctx->startKeyListing("");

    GpgME::Error err;

    while (true){
      GpgME::Key key = ctx->nextKey(err);
      if(err || key.isNull()) break;
      keys.push_back(key);
    }

    ctx->endKeyListing();
  }

  Element recipientMenu::OnRender (){
    std::vector<Element> gpgAccounts;

    for(int i = 0; i < keys.size(); i++) {
      gpgAccounts.push_back(
        vbox({
          hbox({
            text(std::string(" Name: ") + keys[i].userID(0).name() + ' ') | flex,
            separatorHeavy(),
            text(std::string(" Email: ") + keys[i].userID(0).email() + ' ') | flex,
            isRecipientKeySelected(keys[i])? separatorHeavy(), text("[+]") : text("[-]")
          }),
          separatorHeavy(),
          hbox({
            text(std::string(" Fingerprint: ") + keys[i].primaryFingerprint() + ' '),
            separatorHeavy(),
            filler(),
            getTrustLevel(keys[i]) | flex
          })
        }) | borderHeavy | (selected == i? color(Color::Cyan) : isRecipientKeySelected(keys[i])? color(Color::GreenLight) : color(Color::White) | dim)
      );
    }

    Element defaultMessage = vbox({
      paragraphAlignCenter( " ── [!] WARNING ── " ) | color(Color::Yellow),
      separatorDouble(),
      paragraphAlignCenter(" It seems that you either don't have GnuPG installed, \n you haven't yet initialized your keyrings or you \n didn't CREATE/IMPORT your personal GnuPG keys. \n please manually check that you have one. ") | color(Color::Yellow),
      separatorDouble(),
      paragraphAlignCenter(" press ESC/Q to quit. ") | color(Color::Magenta)
    }) | borderDouble | color(Color::Blue) | center;


    Element trustConcern = vbox({
      text(" ── Trust Warning ── ") | bold | color(Color::Yellow) | center,
      separatorDouble(),
      paragraph(" Only trust keys with: ") | size(WIDTH, LESS_THAN, 30),
      text(" • Full") | color(Color::Green),
      text(" • Ultimate") | color(Color::Cyan),
      separator(),
      paragraph(" Avoid keys marked: ") | size(WIDTH, LESS_THAN, 30),
      text(" • Unknown") | color(Color::GrayLight),
      text(" • Never") | color(Color::Red),
      text(" • Marginal") | color(Color::Magenta),
      separator(),
      paragraph(" Always verify finger- \n -prints. out-of-band. ") | size(WIDTH, LESS_THAN, 30)
    }) | borderDouble | color(Color::Blue) | flex;

    Element trustLevelLegend = vbox({
      vbox({
        text(" ── Trust Legend ── ") | bold | color(Color::Yellow) | center,
        separatorDouble(),
        text(" ● Unknown ") | color(Color::GrayLight), 
        text(" ● Undefined ") | color(Color::Yellow), 
        text(" ● Never ") | color(Color::Red), 
        text(" ● Marginal ") | color(Color::Magenta), 
        text(" ● Full ") | color(Color::Green), 
        text(" ● Ultimate ") | color(Color::Cyan), 
        text(" [+] Selected") | color(Color::GreenLight), 
        text(" [-] Unselected ") | color(Color::GrayLight) | dim, 
      }) | borderDouble | color(Color::Blue) | flex,
      trustConcern 
    }) | size(WIDTH, LESS_THAN, 30); 

    Element keysMenu = hbox({
      vbox({
        paragraphAlignCenter(this->HEADER) | color(Color::Yellow),
        separatorDouble(),
        vbox(gpgAccounts),
        separatorDouble(),
        paragraphAlignCenter(this->FOOTER) | color(Color::Magenta)
      }) | borderDouble | color(Color::Blue),
      trustLevelLegend
    }) | center; 
     
    return keys.size() > 0? keysMenu : defaultMessage;
  }
  
  bool recipientMenu::OnEvent (Event e){
    if (e == Event::ArrowUp || e ==  Event::Character('k') || e == Event::Character('K')) {
        if (selected > 0) {
            selected--;
        } else {
            selected = keys.size() - 1;
        }
        return true;
    }

    if (e == Event::ArrowDown || e == Event::Character('j') || e == Event::Character('J')) {
        if (selected < keys.size() - 1) {
            selected++;
        } else {
            selected = 0;
        }
        return true;
    }

    if (e == Event::Escape || e == Event::Character('q') || e == Event::Character('Q')){
      screen->Exit();
      return true;
    }

    if (e == Event::Character('t') || e == Event::Character('T')){
      if(recipients.size() <= keys.size() && keys.size() > 0 && selected >= 0 && selected < keys.size()){

        std::string selectedKeyFP = keys[selected].primaryFingerprint();

        auto it = std::find_if(recipients.begin(), recipients.end(), [&](GpgME::Key key){
            const char * keyFP = key.primaryFingerprint();
            return (keyFP && (selectedKeyFP == keyFP));
        });

        if(it == recipients.end()){
          recipients.push_back(keys[selected]);
        }

      }
      return true;
    }

    if (e == Event::Character('u') || e == Event::Character('U')){
      if(recipients.size() > 0 && keys.size() > 0 && selected >= 0 && selected < keys.size()){
        
        std::string selectedKeyFP = keys[selected].primaryFingerprint();

        auto it = std::find_if(recipients.begin(), recipients.end(), [&](GpgME::Key key){
            const char * keyFP = key.primaryFingerprint();
            return (keyFP && (selectedKeyFP == keyFP));
        });

        if(it != recipients.end()){
          recipients.erase(it);
        }

      }
      
      return true;
    }

    if(e == Event::Return){
      if(!(recipients.size() > 0)){
        this->HEADER = " ── [!] WARNING ── \n" 
                       " You must at least select one recipient key! ";
      }
      else {
        this->HEADER = " Select the recipient (PUBLIC key) accounts to start chatting. \n"
                         " You can select multiple recipients keys. ";
      }
      
      return true;
    }

    return false;
  }

  bool recipientMenu::Focusable() const { return true; }

  std::shared_ptr<std::vector<GpgME::Key>> recipientMenu::getRecipientsKeys(void) const{
    return std::make_shared<std::vector<GpgME::Key>>(recipients);
  }

  Element recipientMenu::getTrustLevel(GpgME::Key key) {
    GpgME::UserID::Validity validity = key.userID(0).validity();
    
    switch (validity) {
        case GpgME::UserID::Unknown:   return text("● Unknown") | color(Color::GrayLight);
        case GpgME::UserID::Undefined: return text("● Undefined") | color(Color::Yellow);
        case GpgME::UserID::Never:     return text("● Never") | color(Color::Red);
        case GpgME::UserID::Marginal:  return text("● Marginal") | color(Color::Magenta);
        case GpgME::UserID::Full:      return text("● Full") | color(Color::Green);
        case GpgME::UserID::Ultimate:  return text("● Ultimate") | color(Color::Cyan);
        default:                       return text("● Invalid") | color(Color::Red);
    }
  }
  
  bool recipientMenu::isRecipientKeySelected (GpgME::Key key){
    if(recipients.size() > 0 && keys.size() > 0 && selected >= 0 && selected < keys.size()){
        
        std::string selectedKeyFP = key.primaryFingerprint();

        auto it = std::find_if(recipients.begin(), recipients.end(), [&](GpgME::Key key){
            const char * keyFP = key.primaryFingerprint();
            return (keyFP && (selectedKeyFP == keyFP));
        });

        if(it != recipients.end()){
          return true;
        }
        else {
          false;
        }
    }
    return false;
  }
  

  /* >=====> Customized Button <=====< */

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

    Component input_;
    Component container_;

    public:
      clientChatScreen(std::shared_ptr<ScreenInteractive>);
      Element OnRender() override;
      bool OnEvent(Event) override;
      bool Focusable() const final;
  };

  clientChatScreen::clientChatScreen(std::shared_ptr<ScreenInteractive> screen_) : screen(screen_) {

    input_ = Input(&INPUT_TEXT, " Enter: ");
    container_ = Container::Vertical({input_, menuButton, message});

    Add(container_);
  }

  Element clientChatScreen::OnRender(){

    Element sideBar = [&] {
      
      if(*toggleMenu){
        *buttonLabel = " Close ";
        return vbox({ 
            hbox({
              paragraphAlignCenter(" ── Menu Options ── ") | flex,
              *toggleMenu ? separatorDouble() : text(""),
              *toggleMenu ? menuButton->Render() : text(""),
            }),
            separatorDouble()
        }) | borderDouble | flex | size(WIDTH, EQUAL, 40);
      }
      else{
        *buttonLabel = " Menu ";
        return text("");
      }
      
    }();

    Element header = hbox({
      paragraphAlignCenter(" ── Server Name ── ") | flex,
      !(*toggleMenu) ? separatorDouble() : text(""),
      !(*toggleMenu) ? menuButton->Render() : text(""),
    });

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

    return hbox({
      vbox({
          vbox({
            header,
            separatorDouble(),
            chatMessages | flex,
          }) | borderDouble | flex,
          vbox({
            input_->Render() | bgcolor(Color::Black) | color(Color::White) | borderDouble
          }) ,
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

