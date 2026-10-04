FROM archlinux:latest AS arc

RUN yes | pacman -Syu

RUN yes | pacman -S git cmake clang ninja gnupg gpgmepp fish 

RUN git clone https://github.com/NinjaTech404/chat-over-gpg-tui.git cyber-chat

WORKDIR cyber-chat

RUN git clone https://github.com/fmtlib/fmt.git ./lib/fmt
RUN git clone https://github.com/ArthurSonzogni/FTXUI.git ./lib/ftxui

RUN cmake -S . -B build -G Ninja -Wauthor
RUN cmake --build build

WORKDIR build

CMD ["fish"]
