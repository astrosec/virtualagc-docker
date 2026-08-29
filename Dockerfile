# Fort Neptune Virtual AGC packaging.
#
# Builds yaAGC (the Apollo Guidance Computer CPU emulator) and yaYUL (the AGC
# assembler) from a pinned Virtual AGC source commit, assembles the mission
# rope images at build time, and produces a small headless runtime image that
# serves the AGC's structured channel I/O on a TCP socket.
#
# The mission software the AGC runs is selectable at runtime (AGC_SOFTWARE),
# with Luminary099 (Apollo 11 LM) as the default. See entrypoint.sh.

ARG BASE_IMAGE=debian:bookworm-slim

# ---------------------------------------------------------------------------
# Stage 1: build yaAGC + yaYUL and assemble the mission ropes.
# ---------------------------------------------------------------------------
FROM ${BASE_IMAGE} AS builder

ARG VIRTUALAGC_REPO=https://github.com/virtualagc/virtualagc
ARG VIRTUALAGC_COMMIT=414514149737a5b2c14bba399f85dd4d96195774
# Space-separated mission directories to assemble with yaYUL.
ARG AGC_MISSIONS="Luminary099 Comanche055"

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates git gcc g++ make libncurses-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
RUN git init virtualagc \
    && cd virtualagc \
    && git remote add origin "${VIRTUALAGC_REPO}" \
    && git fetch --depth 1 origin "${VIRTUALAGC_COMMIT}" \
    && git checkout --detach FETCH_HEAD

WORKDIR /src/virtualagc
RUN make yaAGC yaYUL

RUN set -eu; \
    mkdir -p /opt/agc/roms; \
    manifest=/opt/agc/roms/manifest.json; \
    printf '{\n  "schema_version": "1.0.0",\n' > "$manifest"; \
    printf '  "source_repo": "%s",\n' "${VIRTUALAGC_REPO}" >> "$manifest"; \
    printf '  "source_commit": "%s",\n  "ropes": [\n' "${VIRTUALAGC_COMMIT}" >> "$manifest"; \
    first=1; \
    for m in ${AGC_MISSIONS}; do \
        echo "Assembling ${m}"; \
        ( cd "$m" && ../yaYUL/yaYUL --unpound-page MAIN.agc > MAIN.yul.log ); \
        cp "$m/MAIN.agc.bin" "/opt/agc/roms/${m}.bin"; \
        sha=$(sha256sum "/opt/agc/roms/${m}.bin" | cut -d' ' -f1); \
        [ $first -eq 1 ] || printf ',\n' >> "$manifest"; first=0; \
        printf '    {"mission": "%s", "file": "%s.bin", "sha256": "%s", "assembler": "yaYUL --unpound-page"}' \
            "$m" "$m" "$sha" >> "$manifest"; \
    done; \
    printf '\n  ]\n}\n' >> "$manifest"; \
    cat "$manifest"

# ---------------------------------------------------------------------------
# Stage 2: minimal headless runtime.
# ---------------------------------------------------------------------------
FROM ${BASE_IMAGE}

ARG VIRTUALAGC_REPO=https://github.com/virtualagc/virtualagc
ARG VIRTUALAGC_COMMIT=414514149737a5b2c14bba399f85dd4d96195774
ARG PACKAGING_GIT_COMMIT=unspecified

LABEL org.opencontainers.image.title="Fort Neptune Virtual AGC" \
      org.opencontainers.image.source="https://github.com/astrosec/virtualagc-docker" \
      org.opencontainers.image.revision="${PACKAGING_GIT_COMMIT}" \
      com.astrosec.virtualagc.source_repo="${VIRTUALAGC_REPO}" \
      com.astrosec.virtualagc.source_commit="${VIRTUALAGC_COMMIT}"

RUN apt-get update && apt-get install -y --no-install-recommends libncurses6 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --system --create-home --uid 10001 agc

COPY --from=builder /src/virtualagc/yaAGC/yaAGC /opt/agc/bin/yaAGC
COPY --from=builder /opt/agc/roms/ /opt/agc/roms/
COPY entrypoint.sh /opt/agc/bin/entrypoint.sh
RUN chmod 0755 /opt/agc/bin/entrypoint.sh

USER agc
WORKDIR /home/agc

# yaAGC serves its channel-I/O socket protocol here (yaDSKY2, adapters, tests).
EXPOSE 19697

# The port probe uses bash's /dev/tcp to avoid extra runtime dependencies.
HEALTHCHECK --interval=10s --timeout=3s --start-period=5s \
    CMD bash -c 'exec 3<>"/dev/tcp/127.0.0.1/${AGC_PORT:-19697}"' || exit 1

ENTRYPOINT ["/opt/agc/bin/entrypoint.sh"]
