#include "../esphome/heart_rate.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
  // Catch treating flags as BPM or always decoding a 16-bit value.
  const uint8_t eight_bit[] = {0x00, 145};
  assert(kickr::parse_heart_rate(eight_bit, sizeof eight_bit) == 145);
  // Catch ignoring the format bit or reversing byte order.
  const uint8_t sixteen_bit[] = {0x01, 0x2c, 0x01};
  assert(kickr::parse_heart_rate(sixteen_bit, sizeof sixteen_bit) == 300);
  assert(std::isnan(kickr::parse_heart_rate(nullptr, 0)));
  assert(std::isnan(kickr::parse_heart_rate(eight_bit, 1)));
  assert(std::isnan(kickr::parse_heart_rate(sixteen_bit, 2)));
  // Contact is only meaningful when the sensor supports detecting it.
  const uint8_t no_contact[] = {0x04, 145};
  const uint8_t contact[] = {0x06, 145};
  assert(std::isnan(kickr::parse_heart_rate(no_contact, sizeof no_contact)));
  assert(kickr::parse_heart_rate(contact, sizeof contact) == 145);
  // Optional energy and RR data must not change the BPM offset.
  const uint8_t optional[] = {0x18, 145, 1, 0, 0, 4};
  assert(kickr::parse_heart_rate(optional, sizeof optional) == 145);
  assert(std::isnan(kickr::parse_heart_rate(optional, 3)));
  assert(std::isnan(kickr::parse_heart_rate(optional, 4)));
  assert(std::isnan(kickr::parse_heart_rate(optional, 5)));
  // Synthetic packet: contact detected, 120 bpm, RR interval of 512 ticks.
  const uint8_t synthetic[] = {0x16, 120, 0, 2};
  assert(kickr::parse_heart_rate(synthetic, sizeof synthetic) == 120);
  std::cout << "Heart rate decoder tests passed\n";
}
