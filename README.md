# virtualagc-docker

Reproducible, headless packaging of [Virtual AGC](https://github.com/virtualagc/virtualagc)
(yaAGC, the Apollo Guidance Computer emulator) for the Fort Neptune Apollo 11
mission scenario. This repository is a packaging and deployment aid only — the
mission physics lives in Fort Neptune, and the AGC adapter that couples yaAGC
to the simulation belongs to the Fort Neptune monorepo, not here.

The previous life of this repository — the Hack-a-Sat AGC challenge — is
preserved unchanged on the [`hackasat`](../../tree/hackasat) branch and the
`hackasat-final` tag.

## What the image contains

- **yaAGC**, built from a pinned Virtual AGC source commit (build args
  `VIRTUALAGC_REPO` / `VIRTUALAGC_COMMIT`; the pin is recorded as OCI image
  labels and in the rope manifest).
- **Mission rope images assembled at build time** with yaYUL from the same
  pinned source, with SHA-256s recorded in `/opt/agc/roms/manifest.json`:
  - `Luminary099` — Apollo 11 Lunar Module (the default)
  - `Comanche055` — Apollo 11 Command Module
  The set is controlled by the `AGC_MISSIONS` build arg.
- A non-root runtime, a TCP healthcheck, and nothing else — no GUI, no X11.

## Running

```bash
docker run --rm -p 19697:19697 fortneptune/virtualagc
```

That boots the Apollo 11 LM software (Luminary 099) and serves yaAGC's
structured channel-I/O protocol on port 19697. Connect `yaDSKY2 --host` from a
local Virtual AGC install for an interactive DSKY, or any adapter speaking the
4-byte packet protocol.

### Selecting mission software

The AGC software is flexible with an Apollo 11 default:

| Variable | Meaning | Default |
| --- | --- | --- |
| `AGC_SOFTWARE` | baked-in rope by mission name (`Luminary099`, `Comanche055`) | `Luminary099` |
| `AGC_IMAGE_PATH` | absolute path to a mounted custom rope; overrides `AGC_SOFTWARE` | unset |
| `AGC_PORT` | channel-I/O server port | `19697` |
| `AGC_EXTRA_ARGS` | extra yaAGC arguments | empty |

```bash
# Apollo 11 CM instead of the LM:
docker run --rm -p 19697:19697 -e AGC_SOFTWARE=Comanche055 fortneptune/virtualagc

# A custom rope assembled elsewhere:
docker run --rm -p 19697:19697 \
  -v "$PWD/MyBuild.agc.bin:/roms/custom.bin:ro" \
  -e AGC_IMAGE_PATH=/roms/custom.bin fortneptune/virtualagc
```

Two AGC instances (LM + CM, as the rendezvous phases require) are just two
containers on different host ports.

## Smoke test

`tests/smoke_test.py` is the Track B Phase 1 acceptance gate: it connects to
the channel socket, verifies valid packets on boot, injects the V35E lamp-test
DSKY sequence on channel 015, verifies the AGC responds, and writes every
event as a structured JSON line (no screen scraping anywhere).

```bash
docker run -d --rm --name agc-smoke -p 19697:19697 fortneptune/virtualagc
python3 tests/smoke_test.py 127.0.0.1 19697
docker stop agc-smoke
```

CI runs the same build and smoke test on every push.

## Reproducibility and provenance

- The Virtual AGC source is fetched at an exact commit; changing it is a
  deliberate build-arg change, never drift.
- Rope images are assembled during the build and their SHA-256s recorded in
  `/opt/agc/roms/manifest.json` together with the source repo and commit.
- `PACKAGING_GIT_COMMIT` stamps this repository's commit into
  `org.opencontainers.image.revision`.
- The container always cold-boots (`--no-resume`): a fresh, reproducible
  machine state every run.

## Known limitation: wall-clock pacing

yaAGC free-runs paced to the wall clock. That is fine for interactive use and
for this repository's bring-up role, but the Fort Neptune closed-loop
co-simulation phases require the AGC to advance only under a deterministic
simulation clock — which means embedding yaAGC or stepping it externally, not
running this container's free-running mode. Do not build closed-loop behavior
on top of the wall-clock pacing. (See the Fort Neptune Moon Mission Scenario
document, "yaAGC determinism".)
