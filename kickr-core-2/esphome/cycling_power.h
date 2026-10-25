#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace kickr {

struct Measurement {
  bool valid = false;
  int16_t power = 0;
  bool has_crank = false;
  uint16_t revolutions = 0;
  uint16_t event_time = 0;
};

inline uint16_t le16(const uint8_t *p) {
  return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

// Bluetooth Cycling Power Measurement (1818/2A63), little endian.
inline Measurement parse(const uint8_t *data, size_t size) {
  Measurement result;
  if (size < 4) return result;
  const uint16_t flags = le16(data);
  size_t required = 4;
  // Validate all optional fields, including ones we do not expose.
  const uint8_t bits[] = {0, 2, 4, 5, 6, 7, 8, 9, 10, 11};
  const uint8_t lengths[] = {1, 2, 6, 4, 4, 4, 3, 2, 2, 2};
  for (size_t i = 0; i < sizeof(bits); ++i)
    if (flags & (1u << bits[i])) required += lengths[i];
  if (size < required) return result;

  const uint16_t raw_power = le16(data + 2);
  result.power = raw_power <= 32767 ? int16_t(raw_power)
                                      : int16_t(int32_t(raw_power) - 65536);
  result.valid = true;
  result.has_crank = (flags & (1u << 5)) != 0;
  if (result.has_crank) {
    size_t offset = 4;
    if (flags & (1u << 0)) offset += 1;  // Pedal balance.
    if (flags & (1u << 2)) offset += 2;  // Torque.
    if (flags & (1u << 4)) offset += 6;  // Wheel revolutions.
    result.revolutions = le16(data + offset);
    result.event_time = le16(data + offset + 2);
  }
  return result;
}

class Cadence {
 public:
  void reset() { *this = Cadence(); }

  float update(const Measurement &m, uint32_t now) {
    if (!m.valid || !m.has_crank) {
      reset();
      return NAN;
    }
    // Re-baseline after a notification gap, before the 64s event clock wraps.
    if (!have_previous_ || uint32_t(now - last_packet_) >= 10000) {
      have_previous_ = true;
      previous_revolutions_ = m.revolutions;
      previous_time_ = m.event_time;
      last_packet_ = last_motion_ = now;
      value_ = NAN;
      return value_;
    }
    last_packet_ = now;
    const uint16_t revolutions = uint16_t(m.revolutions - previous_revolutions_);
    const uint16_t ticks = uint16_t(m.event_time - previous_time_);
    if (revolutions != 0 || ticks != 0) {
      previous_revolutions_ = m.revolutions;
      previous_time_ = m.event_time;
      last_motion_ = now;
      const float rpm = ticks ? revolutions * 60.0f * 1024.0f / ticks : NAN;
      // Reject counter resets and implausible cadence; next sample re-baselines.
      value_ = revolutions && rpm <= 250.0f ? rpm : NAN;
    } else if (uint32_t(now - last_motion_) >= 3000) {
      value_ = 0;
    }
    return value_;
  }

 private:
  bool have_previous_ = false;
  uint16_t previous_revolutions_ = 0;
  uint16_t previous_time_ = 0;
  uint32_t last_packet_ = 0;
  uint32_t last_motion_ = 0;
  float value_ = NAN;
};

}  // namespace kickr
