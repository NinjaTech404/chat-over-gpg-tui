#!/bin/env bash
root_dir=$(pwd)
build_dir=${root_dir}/build
checksum=${root_dir}/.log.sha256
logs=${root_dir}/.logs.txt
target=${root_dir}/src/client/main.cpp
# Color variables
COLOR_RESET='\033[0m'
COLOR_GREEN='\033[1;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_RED='\033[1;31m'
COLOR_CYAN='\033[1;36m'
COLOR_MAGENTA='\033[1;35m'
COLOR_BLUE='\033[1;34m'

trap 'echo -e "\n${COLOR_YELLOW}==> Exiting...${COLOR_RESET}"; sleep 0.5; exit 0' INT

hash() {
  local dgst=$(sha256sum $target)
  echo $dgst
}

check_hash_log() {
  if [ -e ${checksum} ]; then
    local old_sum=$(cat $checksum | cut -d ' ' -f1)
    local new_sum=$(hash | cut -d ' ' -f1)
    if [ $old_sum = $new_sum ]; then
      return 1
    else
      sha256sum $target >$checksum
    fi
  else
    sha256sum $target >$checksum
  fi
  return 0
}

main() {
  clear
  echo -e "${COLOR_BLUE}==> launching...${COLOR_RESET}"
  sleep 1
  if [ ! -d $build_dir ]; then
    echo -e "${COLOR_YELLOW}==> creating the cmake environment...${COLOR_RESET}"
    if cmake -S $root_dir -B $build_dir -Wauthor -G Ninja; then
      echo -e "${COLOR_GREEN}==> created the cmake environment! ${COLOR_RESET}"
    else
      echo -e "${COLOR_RED}==> failed to create the cmake environment!!${COLOR_RESET}"
      sleep 0.5
      echo -e "${COLOR_YELLOW}==> see logs.txt for more info.${COLOR_RESET}"
    fi
  fi
  sleep 1
  echo -e "${COLOR_GREEN}==> waiting for new changes...${COLOR_RESET}"
  sleep 1
  while $true; do
    if [ ! -d $build_dir ]; then
      echo -e "${COLOR_YELLOW}==> recreating the cmake environment...${COLOR_RESET}"
      if cmake -S $root_dir -B $build_dir -Wauthor -G Ninja; then
        echo -e "${COLOR_GREEN}==> created the cmake environment! ${COLOR_RESET}"
        sleep 1
        echo -e "${COLOR_GREEN}==> waiting for new changes...${COLOR_RESET}"
      else
        echo -e "${COLOR_RED}==> failed to create the cmake environment!!${COLOR_RESET}"
        sleep 0.5
        echo -e "${COLOR_YELLOW}==> see logs.txt for more info.${COLOR_RESET}"
      fi
    fi
    if check_hash_log; then
      clear
      echo -e "${COLOR_MAGENTA}==> new changes discoverd!${COLOR_RESET}"
      sleep 1
      echo -e "${COLOR_CYAN}==> building...${COLOR_RESET}"
      if cmake --build build; then
        echo -e "${COLOR_GREEN}==> done building!${COLOR_RESET}"
        sleep 1
        echo -e "${COLOR_CYAN}==> running...${COLOR_RESET}"
        sleep 0.5
        ${build_dir}/client 127.0.0.1 8080
        sleep 0.5
        echo -e "\n${COLOR_GREEN}==> waiting for new changes...${COLOR_RESET}"
      else
        echo -e "${COLOR_RED}==> failed building!!${COLOR_RESET}"
        sleep 0.5
        echo -e "${COLOR_YELLOW}==> see logs.txt for more info.${COLOR_RESET}"
        sleep 1
        echo -e "${COLOR_GREEN}==> waiting for new changes...${COLOR_RESET}"
      fi
    fi
    sleep 1
  done
}

main
