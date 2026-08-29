#!/usr/bin/env python3
"""Boot smoke test for the Fort Neptune Virtual AGC container.

Speaks yaAGC's native 4-byte channel-I/O packet protocol (the same structured
interface yaDSKY2 uses — no screen scraping):

    byte0 = 00 u ccccc      (channel bits 8..3; u = counter-request bit)
    byte1 = 01 ccc ddd      (channel bits 2..0; data bits 14..12)
    byte2 = 10 dddddd       (data bits 11..6)
    byte3 = 11 dddddd       (data bits 5..0)

Acceptance (scenario doc, Track B Phase 1):
  1. yaAGC accepts a TCP connection and emits valid channel packets.
  2. A DSKY keypress sequence (V35E, the lamp test) injected on channel 015
     produces AGC output activity in response.
  3. Every exchanged event is logged as a structured JSON line.

Exit code 0 on success, 1 on failure.
"""

import json
import socket
import sys
import time

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 19697
LOG_PATH = sys.argv[3] if len(sys.argv) > 3 else "smoke-events.jsonl"

KEY_CHANNEL = 0o15
# Keycodes from yaDSKY2 (digits 1-9 are 1-9, zero is 16).
KEYS = {"VERB": 17, "NOUN": 31, "ENTR": 28, "RSET": 18, "3": 3, "5": 5}
# RSET first (clears the fresh-start restart lamp, guaranteeing channel
# activity), then V35E, the DSKY lamp test.
KEY_SEQUENCE = ["RSET", "VERB", "3", "5", "ENTR"]

events = []


def log_event(etype, **fields):
    events.append({"time": round(time.monotonic(), 6), "type": etype, **fields})


def form_packet(channel, value):
    return bytes([
        (channel >> 3) & 0x1F,
        0x40 | ((channel << 3) & 0x38) | ((value >> 12) & 0x07),
        0x80 | ((value >> 6) & 0x3F),
        0xC0 | (value & 0x3F),
    ])


def parse_stream(buf):
    """Yield (channel, value) for each well-formed packet; resynchronize on
    signature mismatch. Returns unconsumed tail."""
    out = []
    i = 0
    while i + 4 <= len(buf):
        b = buf[i:i + 4]
        if (b[0] & 0xC0, b[1] & 0xC0, b[2] & 0xC0, b[3] & 0xC0) != (0x00, 0x40, 0x80, 0xC0):
            i += 1  # resync byte-by-byte
            continue
        channel = ((b[0] & 0x1F) << 3) | ((b[1] >> 3) & 7)
        value = ((b[1] << 12) & 0x7000) | ((b[2] << 6) & 0x0FC0) | (b[3] & 0x3F)
        out.append((channel, value))
        i += 4
    return out, buf[i:]


def drain(sock, seconds):
    """Read packets for a period; return list of (channel, value)."""
    got = []
    tail = b""
    deadline = time.monotonic() + seconds
    sock.settimeout(0.25)
    while time.monotonic() < deadline:
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            continue
        if not chunk:
            break
        parsed, tail = parse_stream(tail + chunk)
        for channel, value in parsed:
            got.append((channel, value))
            log_event("agc_output", channel=f"{channel:03o}", value=f"{value:05o}")
    return got


def fail(msg):
    log_event("failure", message=msg)
    write_log()
    print(f"SMOKE FAIL: {msg}", file=sys.stderr)
    sys.exit(1)


def write_log():
    with open(LOG_PATH, "w", encoding="utf-8") as f:
        for seq, e in enumerate(events):
            f.write(json.dumps({"sequence": seq, **e}) + "\n")


def main():
    # 1. Connect (retry while the container starts up).
    sock = None
    for _ in range(40):
        try:
            sock = socket.create_connection((HOST, PORT), timeout=1)
            break
        except OSError:
            time.sleep(0.5)
    if sock is None:
        fail(f"could not connect to yaAGC at {HOST}:{PORT}")
    log_event("connected", host=HOST, port=PORT)

    # 2. yaAGC pushes current non-zero channel state to new clients; a cold
    #    boot may legitimately have nothing non-zero yet, so this is
    #    informational — the hard assertion is the keypress response below.
    boot = drain(sock, 3.0)
    log_event("boot_output", packets=len(boot))

    # 3. Inject the RSET + V35E keypress sequence on channel 015.
    for key in KEY_SEQUENCE:
        sock.sendall(form_packet(KEY_CHANNEL, KEYS[key]))
        log_event("dsky_keypress", key=key, keycode=KEYS[key],
                  channel=f"{KEY_CHANNEL:03o}")
        time.sleep(0.4)

    # 4. Expect AGC output activity in response (lamp test drives the relay
    #    rows on channel 010 and indicator channels).
    response = drain(sock, 5.0)
    if not response:
        fail("no AGC output activity after RSET + V35E keypress sequence")
    relay = [v for c, v in response if c == 0o10]
    log_event("keypress_response_ok", packets=len(response),
              channel_010_updates=len(relay))

    sock.close()
    log_event("success")
    write_log()
    print(f"SMOKE PASS: boot packets={len(boot)}, "
          f"post-V35E packets={len(response)} "
          f"(channel 010 relay updates={len(relay)}); log at {LOG_PATH}")


if __name__ == "__main__":
    main()
