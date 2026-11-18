# syntax=docker/dockerfile:1

# hadolint ignore=DL3029
FROM --platform=linux/amd64 ubuntu:22.04 AS download

ARG VERSION
ARG REPOSITORY=tier4/autoware_lanelet2_map_validator

# hadolint ignore=DL3008
RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates curl unzip \
    && rm -rf /var/lib/apt/lists/*

RUN test -n "${VERSION}" || { echo "Specify the release: docker build --build-arg VERSION=<release, e.g. 1.8.2> ..." >&2; exit 1; } \
    && version="${VERSION#v}" \
    && curl -fsSL -o /tmp/release.zip \
        "https://github.com/${REPOSITORY}/releases/download/${version}/autoware_lanelet2_map_validator-${version}-linux-x86_64.zip" \
    && unzip -q /tmp/release.zip -d /tmp/release \
    && mv /tmp/release/release_package /opt/autoware_lanelet2_map_validator

# hadolint ignore=DL3029
FROM --platform=linux/amd64 ubuntu:22.04

ARG VERSION
LABEL org.opencontainers.image.title="autoware_lanelet2_map_validator" \
      org.opencontainers.image.version="${VERSION}"

# hadolint ignore=DL3008
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        tini \
        libgl1 libegl1 \
        fontconfig fonts-dejavu-core fonts-ipafont-gothic \
        tigervnc-standalone-server openbox novnc \
    && rm -rf /var/lib/apt/lists/*

COPY --from=download /opt/autoware_lanelet2_map_validator /opt/autoware_lanelet2_map_validator

RUN sed -i 's|</applications>|<application class="autoware_lanelet2_map_validator_gui" type="normal"><maximized>yes</maximized></application></applications>|' /etc/xdg/openbox/rc.xml \
    && grep -q autoware_lanelet2_map_validator_gui /etc/xdg/openbox/rc.xml \
    && printf '%s\n' '<!DOCTYPE html>' '<meta http-equiv="refresh" content="0; url=vnc.html?autoconnect=true&resize=scale">' \
        > /usr/share/novnc/index.html \
    && mkdir -p /tmp/.X11-unix \
    && chmod 1777 /tmp/.X11-unix

COPY --chmod=755 <<'EOF' /usr/local/bin/entrypoint
#!/bin/bash
set -eu

GUI=/opt/autoware_lanelet2_map_validator/autoware_lanelet2_map_validator_gui

if [ ! -w "${HOME:-/nonexistent}" ]; then
    HOME="/tmp/home-$(id -u)"
    export HOME
    mkdir -p "$HOME"
fi
XDG_RUNTIME_DIR="/tmp/runtime-$(id -u)"
export XDG_RUNTIME_DIR
mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

run_gui() {
    # The GUI ignores SIGINT/SIGTERM, so exit here to stop the container.
    trap 'exit 130' INT
    trap 'exit 143' TERM
    "$GUI" &
    wait $!
}

gui() {
    if [ -z "${DISPLAY-}" ]; then
        echo "DISPLAY is not set. Pass -e DISPLAY=... or use the web mode (see how_to_use.md)." >&2
        exit 1
    fi
    run_gui
}

web() {
    export DISPLAY=:99
    rm -f /tmp/.X99-lock /tmp/.X11-unix/X99
    Xtigervnc :99 -geometry "${SCREEN_SIZE:-1920x1080}" -depth 24 -desktop autoware_lanelet2_map_validator \
        -SecurityTypes None -localhost -rfbport 5900 -AlwaysShared -nolisten tcp >/dev/null 2>&1 &
    for _ in $(seq 100); do
        [ -S /tmp/.X11-unix/X99 ] && break
        sleep 0.1
    done
    openbox >/dev/null 2>&1 &
    websockify --web /usr/share/novnc 6080 127.0.0.1:5900 >/dev/null 2>&1 &
    echo "Open http://localhost:6080 in your web browser."
    echo "To stop, close the validator window or press Ctrl+C here."
    run_gui
}

case "${1-}" in
"") if [ -n "${DISPLAY-}" ]; then gui; else web; fi ;;
gui) gui ;;
web) web ;;
-*) exec autoware_lanelet2_map_validator "$@" ;;
*) exec "$@" ;;
esac
EOF

ENV PATH=/opt/autoware_lanelet2_map_validator:${PATH} \
    LANG=C.UTF-8 \
    QT_XCB_GL_INTEGRATION=none

WORKDIR /data
EXPOSE 6080
ENTRYPOINT ["/usr/bin/tini", "--", "/usr/local/bin/entrypoint"]
