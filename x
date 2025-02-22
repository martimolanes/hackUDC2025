#!/usr/bin/env bash

set -euo pipefail

ZEPHYR_BASE=~/zephyrproject

function error_command() {
    echo "no such command $command"
    help
}

function help() {
    echo "usage:"
    echo "  run:"
    echo "    $0 build-zephyr     - build Zephyr src"
    echo "    $0 server           - start intiface-engine"
    # echo "  stop:"
    # echo "    $0 stop             - "
}


build_zephyr() {
    echo "Sourcing Zephyr... "
    source $ZEPHYR_BASE/.venv/bin/activate
    echo "Building Zephyr"
    cd ..
    west build -p always -b xiao_ble anov/AnOS
    cd -
}

server() {
    local project_dir="intiface-engine"
    local engine="$project_dir/target/debug/intiface-engine"
    local websocket_port=12345
    local device_config="$HOME/.local/share/com.nonpolynomial.intiface_central/config/buttplug-device-config-v3.json"
    local user_device_config="$project_dir/test.json"
    local log_level="info"

    echo "Starting intiface-engine"

    if [[ ! -x "$engine" ]]; then
        echo "Engine not found or not executable at $engine. Building with cargo..."
        (cd "$project_dir" && cargo build) || { echo "cargo build failed"; return 1; }
        cd ..
    fi

    "$engine" --websocket-port "$websocket_port" \
              --use-bluetooth-le \
              --device-config-file "$device_config" \
              --user-device-config-file "$user_device_config" \
              --log "$log_level"
}

if [[ $PWD != $(git rev-parse --show-toplevel) ]]; then
    echo "This script must be run from the root directory of the repository"
    exit 1
fi

command=${1:-}

case $command in
    build-zephyr ) build_zephyr ;;
    server ) server ;;
    -h) help ;;
    --help) help ;;
    *) error_command ;;
esac

