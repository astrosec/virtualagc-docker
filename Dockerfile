# Hack-a-Sat Apollo Guidance Computer challenge — modernized build.
#
# Same challenge as the 2020 original (preserved at tag hackasat-final):
# the value of PI in Comanche 055's TIME_OF_FREE_FALL.agc is replaced with
# a secret key and the rope reassembled at container start; contestants
# recover the key by reading AGC memory through the DSKY interface.
#
# Modernized: current Debian base, Virtual AGC source pinned to an exact
# commit, minimal build (yaAGC + yaYUL only — connect yaDSKY2 from the
# host instead of X11-forwarding it out of the container), and the AGC
# starts automatically instead of requiring a manual launch.

FROM debian:bookworm-slim

ARG VIRTUALAGC_REPO=https://github.com/virtualagc/virtualagc
ARG VIRTUALAGC_COMMIT=414514149737a5b2c14bba399f85dd4d96195774

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates git gcc g++ gdb make libncurses-dev \
        python3 vim-tiny netcat-openbsd \
    && rm -rf /var/lib/apt/lists/*

RUN git init virtualagc \
    && cd virtualagc \
    && git remote add origin "${VIRTUALAGC_REPO}" \
    && git fetch --depth 1 origin "${VIRTUALAGC_COMMIT}" \
    && git checkout --detach FETCH_HEAD

RUN cd virtualagc && make yaAGC yaYUL

ADD doit.sh /
RUN chmod 0755 /doit.sh

ARG KEY_VAR
ENV DEPLOY_ENV=${KEY_VAR}

ENTRYPOINT ["/doit.sh"]
EXPOSE 19697
