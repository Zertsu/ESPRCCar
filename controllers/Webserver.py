from aiohttp import web
import aiohttp
import socket
import ssl
import queue
from threading import Thread


class UDPhan():
    def __init__(self, bind_addr, port) -> None:
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((bind_addr, port))
        self.socket.settimeout(5)
        self.remote = ("0.0.0.0", 1)
        self.sendQ = queue.Queue()
        self.recvT = Thread(target=self._udp_reciver)
        self.recvT.daemon = True
        self.sendT = Thread(target=self._udp_sender)
        self.sendT.daemon = True
        self.recvT.start()
        self.sendT.start()

    def _udp_reciver(self):
        while True:
            try:
                data, source = self.socket.recvfrom(1024)
                self.remote = source
                print("Data from: {}\t {}".format(source, data))
                if data == b'\x00':
                    self.sendQ.put(b'\x01')
            except socket.timeout:
                if not self.remote[0] == "0.0.0.0":
                    print("No packet from udp")
                    self.sendQ.put(b'\x00')

    def _udp_sender(self):
        while True:
            data = self.sendQ.get()
            if not self.remote[0] == "0.0.0.0":
                self.socket.sendto(data, self.remote)


class Webhan():
    def __init__(self, target_queue) -> None:
        self.targetQ = target_queue
        self.app = web.Application()
        self.clients = ()

        self.app.add_routes([
            web.get('/ws', self.websocket_handler),
            web.get('/', self.hanroot),
            web.static("/", "./data/web/"),
        ])

    def run(self, host="0.0.0.0", port=8080, certfile=None, keyfile=None):
        if not certfile == None and not keyfile == None:
            ssl_context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
            ssl_context.load_cert_chain(certfile, keyfile)
            web.run_app(self.app, ssl_context=ssl_context,
                        host=host, port=port)
        else:
            web.run_app(self.app, host=host, port=port)

    async def websocket_handler(self, request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)
        print("WS Client connected: {}".format(request.remote))
        async for msg in ws:
            if msg.type == aiohttp.WSMsgType.BINARY:
                print(msg.data)
                self.targetQ.put(msg.data)
            elif msg.type == aiohttp.WSMsgType.ERROR:
                print('ws connection closed with exception %s'.format(ws.exception()))

        if ws in self.clients:
            self.clients.remove(ws)
        print("WS Client disconnected: {}".format(request.remote))
        return ws

    async def hanroot(self, request):
        raise web.HTTPFound(location="/index.html")


def main():
    udp = UDPhan("0.0.0.0", 16001)
    web = Webhan(udp.sendQ)
    web.run()


if __name__ == "__main__":
    main()
