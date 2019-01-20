#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

#include <gflags/gflags.h>
#include <boost/format.hpp>

#include <unet/unet.hpp>

using namespace unet;

static const EthernetAddr kHwAddr{{0x06, 0x11, 0x22, 0x33, 0x44, 0x55}};
static const Ipv4Addr kIpv4Addr{10, 255, 255, 102};

static bool validateIpv4Addr(const char* flagname, const std::string& raw) {
  try {
    parseIpv4(raw);
    return true;
  } catch (const Exception&) {
    return false;
  }
}

std::unique_ptr<Stack> stack;

void runApp();

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  (void)validateIpv4Addr;
  stack = std::make_unique<Stack>(std::make_unique<Tap>("tap0"), kHwAddr,
                                  kIpv4Addr);
  runApp();
  return 0;
}