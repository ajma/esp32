#include "../esphome/cycling_power.h"
#include <cassert>
#include <iostream>
#include <vector>

int main() {
  using kickr::parse;
  const std::vector<uint8_t> power = {0, 0, 250, 0};
  assert(parse(power.data(), power.size()).power == 250);
  const uint8_t negative[] = {0, 0, 0xff, 0xff};
  assert(parse(negative, sizeof negative).power == -1);
  assert(!parse(power.data(), 3).valid);
  // All optional fields before crank data: balance, torque, wheel.
  const uint8_t optional[] = {0x35, 0, 200, 0, 50, 1, 0,
                             1, 0, 0, 0, 0, 4, 100, 0, 0, 4};
  auto m = parse(optional, sizeof optional);
  assert(m.valid && m.has_crank && m.revolutions == 100 && m.event_time == 1024);
  for (size_t n = 0; n < sizeof optional; ++n) assert(!parse(optional, n).valid);
  const uint8_t truncated_tail[] = {0x40, 0, 200, 0};
  assert(!parse(truncated_tail, sizeof truncated_tail).valid);
  // Synthetic packet: torque + wheel + crank data, 200 W.
  const uint8_t synthetic[] = {0x34, 0, 0xc8, 0, 1, 0, 2, 0,
                              0, 0, 0, 8, 100, 0, 0, 4};
  const auto decoded = parse(synthetic, sizeof synthetic);
  assert(decoded.valid && decoded.power == 200 && decoded.has_crank);
  assert(decoded.revolutions == 100 && decoded.event_time == 1024);

  kickr::Cadence cadence;
  assert(std::isnan(cadence.update(m, 0)));
  m.revolutions += 2;
  m.event_time += 1024;
  assert(cadence.update(m, 1000) == 120);
  assert(cadence.update(m, 2000) == 120);
  assert(cadence.update(m, 4000) == 0);
  // New crank data after a gap must not imply motion across clock wraps.
  assert(std::isnan(cadence.update(m, 15000)));
  cadence.reset();
  m.revolutions = 65535;
  m.event_time = 65024;
  assert(std::isnan(cadence.update(m, 0)));
  m.revolutions = 0;
  m.event_time = 512;
  assert(cadence.update(m, 1000) == 60);
  m.revolutions = 40000;  // Counter reset/invalid jump.
  assert(std::isnan(cadence.update(m, 2000)));
  m.has_crank = false;
  assert(std::isnan(cadence.update(m, 3000)));
  std::cout << "Cycling power parser and cadence tests passed\n";
}
