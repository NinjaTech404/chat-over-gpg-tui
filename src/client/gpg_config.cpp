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
    
    std::vector<GpgME::Key> keys;
    
    int selected = 0;
    
    std::shared_ptr<ScreenInteractive> screen;
   
    
    public:
      login(std::shared_ptr<ScreenInteractive> screen_);
      std::shared_ptr<GpgME::Key> getSelected();
      Element OnRender() override;
      bool OnEvent(Event e) override;

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

    return vbox({
      paragraphAlignCenter(this->HEADER) | color(Color::Yellow),
      separatorDouble(),
      vbox(gpgAccounts),
      separatorDouble(),
      paragraphAlignCenter(" Use the arows up/down or the k/j buttons to select the key \n press ENTER to confirm or ESC/Q to quit. ") | color(Color::Magenta)
    }) | borderDouble | color(Color::Blue) | center;

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

    return true;
  }


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

    Element trustConcern = vbox({
      text(" --- Trust Warning --- ") | bold | color(Color::Yellow) | center,
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

    return hbox({
      vbox({
        paragraphAlignCenter(this->HEADER) | color(Color::Yellow),
        separatorDouble(),
        vbox(gpgAccounts),
        separatorDouble(),
        paragraphAlignCenter(this->FOOTER) | color(Color::Magenta)
      }) | borderDouble | color(Color::Blue),
      trustLevelLegend
    }) | center; 
     
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

    return true;
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
}

#endif

