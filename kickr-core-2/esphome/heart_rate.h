#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace kickr {
// Bluetooth Heart Rate Measurement (180D/2A37).
inline float parse_heart_rate(const uint8_t *data, size_t size) {
  if (size < 2) return NAN;
  const uint8_t flags = data[0];
  const bool wide = (flags & 0x01) != 0;
  size_t offset = wide ? 3 : 2;
  if (size < offset) return NAN;
  if (flags & 0x08) offset += 2;  // Energy expended.
  if (size < offset) return NAN;
  if ((flags & 0x10) && (size < offset + 2 || (size - offset) % 2 != 0))
    return NAN;
  if ((flags & 0x04) && !(flags & 0x02)) return NAN;  // Lost skin contact.
  return wide ? uint16_t(data[1]) | (uint16_t(data[2]) << 8) : data[1];
}
}  // namespace kickr
