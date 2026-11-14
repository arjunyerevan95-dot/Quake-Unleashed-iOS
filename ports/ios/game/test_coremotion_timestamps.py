"""Check the applied pinned SDL timestamp adapter and application freshness guard."""
from pathlib import Path
import sys
s = (Path(sys.argv[1]) / "src/sensor/coremotion/SDL_coremotionsensor.m").read_text()
assert "SDL_PrivateSensorUpdate(sensor, 0," not in s
for sensor in ("accelerometerData", "gyroData"):
    assert f"(Uint64)({sensor}.timestamp * 1000000.0)" in s
assert "SDL_memcmp(data, sensor->hwdata->data" not in s
app = Path(__file__).with_name("touch_overlay.c").read_text()
assert "||!stamp" in app and "now-gyroFreshTick)<100" in app
print("PASS: real sample timestamps; unchanged samples refresh; stale/zero samples rejected")
