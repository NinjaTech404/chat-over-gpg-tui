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
#include <vector>
#include <memory>
#include <cstddef>

namespace gpgui{

  using namespace ftxui;


  class login : public ComponentBase{
    
    std::string HEADER = " You might select only one GPG account of yours.\n "
                         " NOTE: make sure your PRIVATE/SECRET key is present. ";
    
    std::vector<GpgME::Key> keys;
    
    int selected = 0;
    
    std::shared_ptr<ScreenInteractive> screen;
   
    
    public:
      login(std::shared_ptr<ScreenInteractive> screen_);
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
          text(std::string(" Fingerprint: ") + keys[i].primaryFingerprint())
        }) | borderHeavy | (selected == i? color(Color::Cyan) : color(Color::White))
      );
    }

    return vbox({
      paragraphAlignCenter(this->HEADER) | color(Color::Yellow),
      separatorDouble(),
      vbox(gpgAccounts),
      separatorDouble(),
      paragraphAlignCenter(" Use the arows up/down or the k/j buttons to select the key. ") | color(Color::Magenta)
    }) | borderDouble | color(Color::Blue) | center ;

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
    }

    return true;
  }

}

#endif

