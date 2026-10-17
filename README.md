<div align="center">

<img src="assets/banner.svg" alt="cyber-chat, terminal chat built for Linux" width="100%">

<img src="assets/h-overview.svg" alt="Overview" height="64">

<img src="assets/card-overview.svg" alt="cyber-chat is a terminal chat with end-to-end encryption, built on the OpenPGP protocol, written in C++20 with FTXUI." width="840">

<img src="assets/divider.svg" alt="" width="100%">

<img src="assets/h-parts.svg" alt="Parts" height="64">

<table>
<tr>
<td align="center" valign="top"><img src="assets/card-client.svg" alt="client: the FTXUI terminal interface, encrypting and decrypting through GPGME++." width="410"></td>
<td align="center" valign="top"><img src="assets/card-server.svg" alt="server: accepts connections with Asio and shows its status in an FTXUI screen." width="410"></td>
</tr>
</table>

<img src="assets/divider.svg" alt="" width="100%">

<img src="assets/h-build.svg" alt="Build" height="64">

<img src="assets/card-build.svg" alt="Builds with CMake and Ninja, with an Arch Linux Dockerfile and CPack packaging." width="840">

</div>

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

Start the server with an IP address, a port and a server name:

```bash
server <ip> <port> <server name>
```

<div align="center">

<img src="assets/footer.svg" alt="End-to-end encrypted" width="100%">

</div>
