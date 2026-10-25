"""List nearby BLE advertisements; never connect or write to a device."""
import asyncio
import json
from pathlib import Path

from bleak import BleakScanner


async def main():
    print("Scanning BLE advertisements for 15 seconds...", flush=True)
    devices = await BleakScanner.discover(timeout=15, return_adv=True)
    rows = []
    for device, adv in devices.values():
        rows.append({
            "name": adv.local_name or device.name,
            "address": device.address,
            "rssi": adv.rssi,
            "services": adv.service_uuids,
            "manufacturer_data": {str(k): v.hex() for k, v in adv.manufacturer_data.items()},
            "service_data": {k: v.hex() for k, v in adv.service_data.items()},
        })
    rows.sort(key=lambda row: row["rssi"], reverse=True)
    path = Path("/tmp/kickr-ble-scan.json")
    path.write_text(json.dumps(rows, indent=2))
    for row in rows:
        print(json.dumps(row), flush=True)
    print(f"Saved {len(rows)} devices to {path}", flush=True)


if __name__ == "__main__":
    asyncio.run(main())
