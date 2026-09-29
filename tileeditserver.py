from fastapi import FastAPI
import uvicorn
from fastapi.staticfiles import StaticFiles
import socket

app = FastAPI()

app.mount("/", StaticFiles(directory="render/tiles", html=True), name="static")


def get_local_ip():
    hostname = socket.gethostname()
    return socket.gethostbyname(hostname)


if __name__ == "__main__":
    ip = get_local_ip()

    print(f"\nServer running at:")
    print(f"  Local:   http://127.0.0.1:8000")
    print(f"  Network: http://{ip}:8000\n")

    uvicorn.run(app, host="0.0.0.0", port=8000)