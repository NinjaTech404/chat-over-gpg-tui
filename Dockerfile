FROM archlinux:latest AS arc

RUN yes | pacman -Syu

RUN yes | pacman -S git cmake clang ninja gnupg gpgmepp fish 

RUN git clone https://github.com/NinjaTech404/chat-over-gpg-tui.git cyber-chat

WORKDIR cyber-chat

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build
RUN cmake --install build

WORKDIR build

CMD ["fish"]
