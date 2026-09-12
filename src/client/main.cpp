#include <client/main.hpp>
using namespace ftxui;


auto io = std::make_shared<asio::io_context>();
auto sock = std::make_shared<tcp::socket>(*io);

auto ui = std::make_shared<ScreenInteractive>(ScreenInteractive::Fullscreen());

std::shared_ptr<GpgME::Key> clientAccount = std::make_shared<GpgME::Key>();
std::shared_ptr<std::string> passphrase = std::make_shared<std::string>();

std::shared_ptr<Components> messages = std::make_shared<Components>();

std::thread tcp_connection (const char* ip_address, const char* port){
  tcp::endpoint ep (asio::ip::make_address(ip_address), std::stoi(port));
  return std::thread([ep]{
    client::connect(io, sock, ep, []{
      client::read_loop(io, sock, [](std::string data){

        try{
          json json_data = client::receive_data(data, clientAccount, passphrase);
          Component message = std::make_shared<screens::Message>(
          json_data["name"].get<std::string>(),
          json_data["fingerprint"].get<std::string>(),
          json_data["date"].get<std::string>(),
          json_data["status"].get<int>(),
          json_data["data"].get<std::string>()
        );

        messages->push_back(message);

        ui->PostEvent(Event::Custom);

        }
        catch(const std::runtime_error& err){}

        Component message = std::make_shared<screens::Message>("test", "test", "test", 200, data);
        messages->push_back(message);

      });
    });
    io->run();
  });
}

enum class Screens : char { Login = 0, RecipientMenu = 1, ClientScreen = 2, PassphrasePrompt = 3, ErrorMessage = 4};

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        return 1;
    }

    std::thread connect_to_server = tcp_connection(argv[1], argv[2]);
      
    int currentScreen = static_cast<int>(Screens::Login);

    std::shared_ptr<std::vector<GpgME::Key>> recipientKeys =  std::make_shared<std::vector<GpgME::Key>>();
  

    std::shared_ptr<std::function<void()>> error_message_handler = std::make_shared<std::function<void()>>();
    std::shared_ptr<std::string> error_message = std::make_shared<std::string>();

    std::shared_ptr<gpg_screens::login> login = std::make_shared<gpg_screens::login>(ui);
    std::shared_ptr<gpg_screens::recipientMenu> recipientUI = std::make_shared<gpg_screens::recipientMenu>(ui);
    std::shared_ptr<screens::clientChatScreen> clientChat = std::make_shared<screens::clientChatScreen>(io, sock, ui, messages, clientAccount, recipientKeys);
    std::shared_ptr<screens::ErrorMessage> errorUI = std::make_shared<screens::ErrorMessage>(ui, error_message, error_message_handler);


    std::shared_ptr<screens::PassphrasePrompt> passphrasePrompt = std::make_shared<screens::PassphrasePrompt>(ui, passphrase, [&]{
      try{
        if(gpg::is_passphrase_correct(clientAccount, passphrase)){
          currentScreen = static_cast<int>(Screens::RecipientMenu);
        }
      } catch(const std::runtime_error& err){
        *error_message = err.what();
        *error_message_handler = [&]{
          currentScreen = static_cast<int>(Screens::PassphrasePrompt);
        };
        currentScreen = static_cast<int>(Screens::ErrorMessage);
      }
    });
    

    Component loginWrapper = CatchEvent(login, [&](Event e){
      if(e == Event::Return){
       
        try {

          *clientAccount = *login->getSelected();
          if(clientAccount){
            if(gpg::can_key_decrypt(*clientAccount)){
              currentScreen = static_cast<int>(Screens::PassphrasePrompt);
            }
          }

        }
        catch (const std::runtime_error& err) {
          *error_message = err.what();
          *error_message_handler = [&]{
            currentScreen = static_cast<int>(Screens::Login);
          };
          currentScreen = static_cast<int>(Screens::ErrorMessage);
        }
  

        return false;
      }
      return false;

    });

    Component recipientUIWrapper = CatchEvent(recipientUI, [&](Event e){
      if(e == Event::Return){

        *recipientKeys = *recipientUI->getRecipientsKeys();
        if(recipientKeys && recipientKeys->size() > 0){

          for(auto& key : *recipientKeys){
            try{
              if(gpg::can_key_encrypt(key)){
                currentScreen = static_cast<int>(Screens::ClientScreen);
              }

            }
            catch (const std::runtime_error& err){
              *error_message = err.what();
              *error_message_handler = [&]{
                currentScreen = static_cast<int>(Screens::RecipientMenu);
              };
              currentScreen = static_cast<int>(Screens::ErrorMessage);
            }
          }
          

        }

        return false;
      }
      return false;

    });

    std::vector<Component> screensUI = {loginWrapper, recipientUIWrapper, clientChat, passphrasePrompt, errorUI};

    Component tab_container = Container::Tab(screensUI, &currentScreen);


    ui->Loop(tab_container);
    connect_to_server.join();
    return 0;
}
