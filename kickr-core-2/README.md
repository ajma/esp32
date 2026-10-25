# KICKR CORE 2 → Home Assistant

An ESPHome configuration for an ESP32-C3 that reads a Wahoo KICKR CORE 2 over
Bluetooth and publishes power, cadence, and bridged heart rate to Home Assistant.
The board also remains a general Home Assistant Bluetooth proxy.

## Hardware and software

- An ESP32-C3 development board with 4 MB flash and USB power.
- A KICKR CORE 2 within Bluetooth range.
- A 2.4 GHz Wi-Fi network and Home Assistant with the ESPHome integration.
- ESPHome Device Builder or CLI, version 2026.10.0 or newer.
- For heart rate, a compatible monitor paired through KICKR Bridge.

No GPIO wiring or MQTT broker is needed. The configuration subscribes to
measurement notifications without sending trainer control commands.

## Setup

The main configuration is [kickr-core-2.yaml](kickr-core-2.yaml). It is
self-contained and does not need the C++ headers in `esphome/`.

1. Copy `esphome/secrets.yaml.example` to `secrets.yaml` beside the main YAML,
   then fill in your Wi-Fi credentials and API encryption key. If updating an
   existing proxy, reuse its current API key.
2. Set `name` and `friendly_name` for your board. The defaults, `pain-cave` and
   **Pain Cave**, are examples. Retain an existing proxy's name when updating
   it in place.
3. Replace `kickr_mac` with your trainer's Bluetooth MAC address. You can obtain
   it with a BLE scanner. For discovery through ESPHome, set `kickr_name` to
   the trainer's advertised name, install once with the placeholder MAC, and
   look for `Trainer Bluetooth MAC:` in Logs. Copy that address into `kickr_mac`
   and install again.
4. Paste the YAML into ESPHome Device Builder and configure the same secrets
   there, or use the CLI commands below. Existing ESPHome devices can update
   over Wi-Fi; use USB for a first installation.
5. Add the discovered ESPHome device in Home Assistant under **Settings →
   Devices & services**. If discovery fails, add it using the ESP32's IP address.
6. Pedal and compare the Home Assistant readings against your riding app. Check
   that readings recover after restarting the trainer or bridge.

To generate an API key for a new device:

```sh
python3 -c 'import base64,secrets; print(base64.b64encode(secrets.token_bytes(32)).decode())'
```

Run these commands from this directory after filling in `secrets.yaml`:

```sh
esphome config kickr-core-2.yaml
esphome compile kickr-core-2.yaml
```

To build and install over the network:

```sh
esphome run kickr-core-2.yaml --device DEVICE_IP
```

For bridged heart rate, wear the monitor and use **Wahoo app → CORE 2 → KICKR
Bridge → Add Sensor**. Zwift manages its bridged sensors separately. See
[Wahoo's bridge instructions](https://support.wahoofitness.com/hc/en-us/articles/29755573392786-How-to-use-the-KICKR-Bridge).

## Exposed entities and behavior

| Entity | Bluetooth source |
| --- | --- |
| KICKR Power | Cycling Power service `1818`, characteristic `2A63` |
| KICKR Cadence | Fitness Machine service `1826`, Indoor Bike Data `2AD2` |
| KICKR Heart rate | Heart Rate service `180D`, characteristic `2A37` |
| KICKR connected | Trainer BLE connection state |

All three measurements share one connection to the trainer. Heart rate comes
through the CORE 2, so the ESP32 does not connect directly to the monitor.
Readings become unknown after ten seconds without data or on disconnect.
“Connected” describes the Bluetooth link; it does not mean someone is riding.

Two additional active BLE connection slots are available to Home Assistant's
Bluetooth proxy, with three total connections configured. Advertisement
forwarding does not consume these slots. Update the trainer firmware and close
unused device connections if the bridge cannot connect alongside your riding app.

Speed, distance, and controller buttons are not exposed. Virtual speed and
distance in a riding app can differ from trainer measurements.

## Verification and limitations

The configuration was checked with ESPHome 2026.10.0, and the self-contained YAML
passed a full ESP32-C3 firmware build. That build used 73.6% of the app partition
and 42.9% static RAM. Dynamic memory usage and radio stability need checking on
the actual board. The three Bluetooth measurement streams were also observed
through a separate connection to a CORE 2 running firmware 2.5.37.

The `esphome/` directory retains an alternative configuration using C++ headers.
It derives cadence from crank counters in Cycling Power notifications; the main
YAML reads cadence directly from Indoor Bike Data. For the alternative, keep
its headers and a separate `secrets.yaml` beside `esphome/kickr.yaml`.

Host-side tests use synthetic packets and cover the alternative C++ decoders,
including optional fields, truncated packets, signed watts, cadence calculation,
stale gaps, and counter wraparound. They do not directly exercise the main
YAML's inline lambdas or replace firmware and hardware checks.

```sh
c++ -std=c++11 -Wall -Wextra -Werror -pedantic tests/cycling_power_test.cpp -o /tmp/kickr-test
/tmp/kickr-test
c++ -std=c++11 -Wall -Wextra -Werror -pedantic tests/heart_rate_test.cpp -o /tmp/kickr-hr-test
/tmp/kickr-hr-test
```

## Optional Bluetooth inspection tools

From a Bluetooth-enabled computer with `uv` installed:

```sh
uv run --python 3.12 --with bleak==3.0.2 python tools/scan_ble.py
uv run --python 3.12 --with bleak==3.0.2 python tools/inspect_kickr.py --name "KICKR CORE XXXX"
```

Replace the example trainer name with your own. The inspection script connects
and subscribes to standard measurement characteristics, reads basic device
information, then disconnects. It sends no trainer control commands.

macOS requires Bluetooth permission and reports a Mac-specific peripheral UUID
rather than the MAC address needed by ESPHome. Use an ESP32 scan or another
scanner that exposes hardware addresses to get that address.

The tools save captures under `/tmp`. Captures can contain device identifiers
and heart-rate measurements; they are not included in this repository.

## References

- [ESPHome BLE client sensors](https://esphome.io/components/sensor/ble_client/)
- [ESPHome Bluetooth proxy](https://esphome.io/components/bluetooth_proxy/)
- [ESPHome ESP32 configuration](https://esphome.io/components/esp32/)
- [Wahoo connection support](https://support.wahoofitness.com/hc/en-us/articles/360014828799-Which-Wahoo-products-offer-Multiperipheral-Bluetooth-connections)
- [Bluetooth SIG GATT field definitions](https://btprodspecificationrefs.blob.core.windows.net/gatt-specification-supplement/GATT_Specification_Supplement.pdf)
