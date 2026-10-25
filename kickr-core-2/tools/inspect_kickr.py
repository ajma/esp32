"""Inspect a named trainer and subscribe only to standard measurement data.

No control-point or proprietary characteristic writes are performed.
"""
import argparse
import asyncio
import json
from pathlib import Path

from bleak import BleakClient, BleakScanner

MEASUREMENTS = {"2a63", "2ad2", "2a37", "2a5b"}
READABLE_INFO = {"2a24", "2a26", "2a29", "2a19"}


async def main(target):
    device = await BleakScanner.find_device_by_name(target, timeout=15)
    if device is None:
        raise RuntimeError(f"{target} not advertising")
    rows, packets = [], []
    async with BleakClient(device, timeout=20) as client:
        print(f"Connected to {device.name}; discovering services", flush=True)
        for service in client.services:
            print(f"SERVICE {service.uuid} {service.description}", flush=True)
            for char in service.characteristics:
                row = {"service": service.uuid, "uuid": char.uuid,
                       "description": char.description, "properties": char.properties}
                rows.append(row)
                print(json.dumps(row), flush=True)
                short = char.uuid[4:8]
                if short in READABLE_INFO and "read" in char.properties:
                    try:
                        raw = await asyncio.wait_for(client.read_gatt_char(char), timeout=5)
                        row["value_hex"] = raw.hex()
                        row["value_text"] = raw.decode(errors="replace")
                        print(f"  INFO {short}: {row['value_text']!r} ({raw.hex()})", flush=True)
                    except Exception as exc:
                        print(f"  Read failed: {exc}", flush=True)
                if short in MEASUREMENTS and "notify" in char.properties:
                    def callback(sender, data):
                        packet = {"uuid": sender.uuid, "hex": data.hex()}
                        packets.append(packet)
                        if sum(p["uuid"] == sender.uuid for p in packets) <= 5:
                            print(f"  MEASUREMENT {sender.uuid}: {data.hex()}", flush=True)
                    try:
                        await asyncio.wait_for(client.start_notify(char, callback), timeout=5)
                    except Exception as exc:
                        print(f"  Subscribe failed: {exc}", flush=True)
        print("Listening to measurement notifications for 20 seconds...", flush=True)
        await asyncio.sleep(20)
    Path("/tmp/kickr-gatt.json").write_text(json.dumps({"services": rows, "packets": packets}, indent=2))
    print(f"Disconnected. Saved {len(packets)} measurements to /tmp/kickr-gatt.json", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--name", required=True, help="Trainer's advertised Bluetooth name")
    args = parser.parse_args()
    asyncio.run(main(args.name))
