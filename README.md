# ESP32 projects

ESPHome configurations and supporting tools for ESP32 devices connected to Home
Assistant. Each project has its own folder, configuration, and setup notes.

| Project | Configuration | Description |
| --- | --- | --- |
| [KICKR CORE 2](kickr-core-2/README.md) | [kickr-core-2.yaml](kickr-core-2/kickr-core-2.yaml) | Power, cadence, and bridged heart rate, alongside a Home Assistant Bluetooth proxy |

## Getting started

Install the ESPHome command-line tools with [uv](https://docs.astral.sh/uv/):

```sh
uv tool install --python 3.12 esphome==2026.10.0
```

Open a project's README for hardware requirements and device-specific setup.
For the KICKR CORE 2 bridge:

```sh
cd kickr-core-2
cp esphome/secrets.yaml.example secrets.yaml
```

Fill in `secrets.yaml` with your Wi-Fi credentials and API encryption key, then
customize the device name and trainer Bluetooth address in `kickr-core-2.yaml`.
Reuse the API key when updating an existing ESPHome device.

Validate and build without flashing:

```sh
esphome config kickr-core-2.yaml
esphome compile kickr-core-2.yaml
```

To build and install on a device over the network:

```sh
esphome run kickr-core-2.yaml --device DEVICE_IP
```

The self-contained YAML can also be pasted into Home Assistant's ESPHome Device
Builder, with its secrets configured there.

## Repository layout

```text
kickr-core-2/
  kickr-core-2.yaml   # Self-contained ESPHome configuration
  README.md          # Hardware, setup, and Bluetooth findings
  esphome/           # Earlier configuration, C++ decoders, secrets example
  tests/             # Host-side decoder checks
  tools/             # Bluetooth discovery and inspection scripts
```

Add future devices in separate folders with a README and a primary ESPHome YAML.
Keep project-specific helpers and tests in the same folder.

## Credentials and build files

Store Wi-Fi passwords and API keys in a local `secrets.yaml` beside the
configuration that uses them. Secrets, generated `.esphome` build files, and
Python virtual environments are ignored by Git. Commit example secrets with
placeholders so another machine can set up the project.
