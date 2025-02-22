import asyncio
import websockets

async def handle_connection(websocket):
    print(f"Client connected: {websocket.remote_address}")
    try:
        async for message in websocket:
            print(f"Received: {message}")
            response = f"Server received: {message}"
            await websocket.send(response)
    except websockets.exceptions.ConnectionClosed:
        print(f"Client disconnected: {websocket.remote_address}")

async def main():
    async with websockets.serve(handle_connection, "localhost", 8765):
        print("WebSocket server started at ws://localhost:8765")
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    asyncio.run(main())
