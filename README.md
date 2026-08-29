# Hack-a-Sat Apollo Guidance Computer Challenge

The AGC challenge from Hack-a-Sat, kept workable on modern systems. The
original 2020 files are preserved unchanged at the `hackasat-final` tag; this
branch is the same challenge on a current base image with a pinned Virtual AGC
source and an auto-starting AGC.

## The challenge

At container start, the value of PI in Comanche 055's `TIME_OF_FREE_FALL.agc`
is replaced with a secret key (`3.141592653` becomes `3.<key>`), PI's memory
address is shifted by a random number of decoy constants, and the rope is
reassembled with yaYUL. The container then boots the real Apollo 11 Command
Module software on yaAGC and serves the DSKY channel protocol on port 19697.

Contestants connect a DSKY, locate PI in erasable/fixed memory, read out the
AGC double-precision octal words, and convert them back to recover the key.
The solver utilities in this repository (`float-conv-agc`, `float-convert`,
`get-double-fromagc`, `read-agc-memory`) demonstrate the conversions.

## Run it

```sh
./start-challenge.sh            # random key
./start-challenge.sh 987654321  # chosen key
```

The script builds `float-conv-agc`, prints the key and the AGC octal
representation contestants should find (the expected solution), builds the
image, and starts the container with the AGC listening on port 19697.

Connect a DSKY from your host (no X11 forwarding needed):

```sh
yaDSKY2 --host=127.0.0.1 --port=19697
```

For the original interactive-shell behavior (patch the ROM but start a shell
instead of the AGC):

```sh
docker run --rm -it -p 19697:19697 -e DEPLOY_ENV=987654321 \
  -e CHALLENGE_SHELL=1 hackasat-agc
```

## Notes on the modernization

- Base image ubuntu:16.04 → debian:bookworm-slim; the dotdeb/allegro/wx/GTK
  setup is gone because only yaAGC and yaYUL are built (the GUI tools were
  never needed inside the container — connect yaDSKY2 remotely).
- The Virtual AGC source is fetched at a pinned commit instead of an
  unpinned clone, so the challenge builds the same way every time.
- `--security-opt seccomp=unconfined` and the `unshare` invocation are no
  longer needed (they worked around the allegro joystick driver, which is no
  longer present).
- The missing `float-conv-agc` Makefile rule is fixed, and the build uses
  `cc` rather than hardcoded clang.
- The reference copy of the original patched `TIME_OF_FREE_FALL.agc` remains
  in the repository root.
