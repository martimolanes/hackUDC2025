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
    echo "    $0 web              - start playground web toy server"
}


build_zephyr() {
    echo "Sourcing Zephyr... "
    source $ZEPHYR_BASE/.venv/bin/activate
    echo "Building Zephyr"
    cd ..
    west build -p always -b xiao_ble anov/AnOS
    cd -
    echo "\n now mount the device, with gnome-files, and flash it with:"
    echo "  \$ cd .. && west flash -r uf2"
}

server() {
    bluetoothctl disconnect E2:40:B8:D4:8D:1E || true
    bluetoothctl scan on
    local project_dir="intiface-engine"
    local engine="$project_dir/target/debug/intiface-engine"
    local websocket_port=12345
    local device_config="$project_dir/config/buttplug-device-config-v3.json"
    local user_device_config="$project_dir/config/test.json"
    local log_level="trace"

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
              --log "$log_level" \
              --websocket-use-all-interfaces
}

web() {
    echo "Starting buttplug-playground web toy server"
    cd buttplug-playground
    source /usr/share/nvm/init-nvm.sh
    nvm use 16 || nvm install 16 && nvm use 16
    yarn install
    yarn start:remote --env development
    cd -
}

if [[ $PWD != $(git rev-parse --show-toplevel) ]]; then
    echo "This script must be run from the root directory of the repository"
    exit 1
fi

command=${1:-}

case $command in
    build-zephyr ) build_zephyr ;;
    server ) server ;;
    web ) web ;;
    -h) help ;;
    --help) help ;;
    *) error_command ;;
esac

