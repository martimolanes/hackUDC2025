import websocket
import threading
import readline
import json
import time

import subprocess

START_CONNECTION = [{"RequestServerInfo":{"Id":1,"ClientName":"Buttplug Playground","MessageVersion":2}}]
REQUEST_DEV = [{"RequestDeviceList":{"Id":2}}]
START_SCAN = [{"StartScanning":{"Id":3}}]
idx = 4

TIME_BETWEEN_HITS = 0.5
TIME_BETWEEN_SQUARES = 2

DURATION_HIT = 0.25

def get_best_move(moves, stockfish_path='stockfish', movetime=1000):
    """
    Get the best move from Stockfish given a list of moves from the starting position.

    Args:
        moves (list): List of UCI-formatted moves (e.g., ["e2e4", "e7e5"]).
        stockfish_path (str): Path to the Stockfish executable. Defaults to 'stockfish'.
        movetime (int): Time in milliseconds for Stockfish to calculate the best move.

    Returns:
        str: The best move in UCI format, or None if not found.
    """
    proc = subprocess.Popen(
        [stockfish_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

    try:
        # Initialize UCI mode
        proc.stdin.write("uci\n")
        proc.stdin.flush()
        while True:
            line = proc.stdout.readline()
            if 'uciok' in line:
                break

        # Check readiness
        proc.stdin.write("isready\n")
        proc.stdin.flush()
        while True:
            line = proc.stdout.readline()
            if 'readyok' in line:
                break

        # Set up position and calculate best move
        position_cmd = f"position startpos moves {' '.join(moves)}\n"
        proc.stdin.write(position_cmd)
        proc.stdin.write(f"go movetime {movetime}\n")
        proc.stdin.flush()

        # Extract best move
        bestmove = None
        while True:
            line = proc.stdout.readline()
            if line.startswith('bestmove'):
                parts = line.strip().split()
                if len(parts) >= 2:
                    bestmove = parts[1]
                break
        return bestmove
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=0.1)
        except subprocess.TimeoutExpired:
            proc.kill()


def on_message(ws, message):
    print(f"\nReceived: {message}\n> ", end="")

def send_messages():
    global ws

    time.sleep(1)

    ws.send(json.dumps(START_CONNECTION))
    ws.send(json.dumps(REQUEST_DEV))
    ws.send(json.dumps(START_SCAN))

    all_messages = []
    while True:
        message = input("> ")
        all_messages.append(message)
        best_move = get_best_move(all_messages)
        print("STOCKFISH ::", best_move)
        all_messages.append(best_move)
        send_best_move(ws, best_move)

def send_best_move(ws, best_move):
    squares = [best_move[:2], best_move[2:]]

    for square in squares:
        letter = ord(square[0]) - ord('a') + 1
        number = ord(square[1]) - ord('0')

        vibrate_times(ws, letter)
        time.sleep(TIME_BETWEEN_SQUARES)
        vibrate_times(ws, number)
        time.sleep(TIME_BETWEEN_SQUARES)

def vibrate_times(ws, times):
    global idx

    for i in range(times):
        ok_command = [ { "VibrateCmd": { "Id": idx, "DeviceIndex": 0, "Speeds": [ { "Index": 0, "Speed": 0.30 } ] } } ]
        idx += 1
        stop_command = [ { "VibrateCmd": { "Id": idx, "DeviceIndex": 0, "Speeds": [ { "Index": 0, "Speed": 0.00 } ] } } ]
        idx += 1

        ws.send(json.dumps(ok_command))
        time.sleep(DURATION_HIT)
        ws.send(json.dumps(stop_command))
        time.sleep(TIME_BETWEEN_HITS)

def start_websocket():
    global ws
    # ws = websocket.WebSocketApp("ws://localhost:8765",
    ws = websocket.WebSocketApp("ws://10.20.26.206:12345",
                              on_message=on_message)
    ws.run_forever()

if __name__ == "__main__":
    print("Connecting to WebSocket...")
    ws_thread = threading.Thread(target=start_websocket, daemon=True)
    ws_thread.start()

    try:
        send_messages()
    except KeyboardInterrupt:
        print("\nClosing connection...")
