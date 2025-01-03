import logging
import socket
import threading
import time

logging.basicConfig(level=logging.DEBUG,encoding='utf-8')
log = logging.getLogger(__name__)

class DummyAIOIServer:
    def __init__(self, deviceName: str, port: int, interruptDelayS: int = 1):
        self.host = "127.0.0.1"
        self.port = port
        self.deviceName = deviceName
        self.interruptDelayS = interruptDelayS
        self.interruptAddressSeqMap: dict[str, int] = {}
        self.serverSequence = 1
        self._shutdown: threading.Event = threading.Event()
        self._isConnected: bool = False
        self._pendingInterruptAddressRequests: set[str] = set()
        self.thread = threading.Thread(target=self._RunServer, daemon=True)
        self.thread.start()

    @property
    def isConnected(self) -> bool:
        return self._isConnected

    def SendInterruptResponse(self, addressInt: int) -> bool:
        address = "{:04d}".format(addressInt)
        if address not in self.interruptAddressSeqMap or address in self._pendingInterruptAddressRequests:
            # log.warn(f"[{self.deviceName}] {address} not in interruptAddressSeqMap: {self.interruptAddressSeqMap}")
            return False
        self._pendingInterruptAddressRequests.add(address)
        return True

    def _InterruptResponse(self, conn: socket.socket, seq: int, address: str):
        # TODO: use async instead of time sleep
        # time.sleep(self.interruptDelayS)
        # if self.deviceName == "WASHER_TANK_PPS":
        #     response = "\x02"+"0000006t91010"+"\x03"
        # else:
        #     response = "\x02" + "{:03d}".format(seq) + "0013t" + address + "23" + address + "00\x03"
        response = "\x02" + "{:03d}".format(seq) + "0007t" + address + "00\x03"
        conn.send(response.encode())
        log.debug(f"[{self.deviceName}] AIOI server Sent interrupt response: {response}")

    def _RunServer(self):
        interval = 0.0
        while not self._shutdown.wait(interval):
            try:
                interval = 10.0
                # Create a socket
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
                    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                    server_socket.bind((self.host, self.port))
                    server_socket.listen(5)
                    server_socket.settimeout(10.0)
                    # spamming when rb is down
                    log.debug(f"{self.deviceName}: Server started on {self.host}:{self.port}. Waiting for connections...")
                    try:
                        conn, addr = server_socket.accept()
                        with conn:
                            log.info(f"[{self.deviceName}] AIOI server Accepted connection from {addr}")
                            self._isConnected = True
                            self._handle_client(conn)
                    except socket.timeout:
                        interval = 5.0
                    except Exception as e:
                        log.debug(f"Error with client connection: {e}, try to reconnect")
                        interval = 5.0
            except Exception as e:
                log.debug(f"Unexpected error: {e}. Restarting server...")
                interval = 5.0  # Wait before restartingWait before retrying
            self._isConnected = False

    def _handle_client(self, conn: socket.socket):
        try:
            conn.setblocking(False)
            while not self._shutdown.is_set():
                if self._pendingInterruptAddressRequests and (address := self._pendingInterruptAddressRequests.pop()) in self.interruptAddressSeqMap:
                    self._InterruptResponse(conn, self.interruptAddressSeqMap[address], address)
                try:
                    data = conn.recv(1024)
                    if not data:
                        log.debug(f"{self.deviceName}: No data, client might be disconnected.")
                        return
                except ConnectionResetError:
                    log.debug(f"{self.deviceName}: Client abruptly disconnected.")
                    return
                except BlockingIOError:
                    continue
                except Exception as e:
                    log.warn(f"Failed to receives because {e}")
                    return
                if not data:
                    continue
                msg = data.decode()
                messages = msg.split('\x03')
                log.debug(f"[{self.deviceName}] AIOI server Received: {messages}")
                for message in messages:
                    if message == "":
                        continue
                    if "O" not in message:
                        sequence = message[1:4]
                        response = "\x02" + sequence + "0001" + "o" + "\x03"
                        conn.send(response.encode())
                        # log.debug(f"[{self.deviceName}] AIOI server Sent: {response}")
                    if "PP505" in message:
                        address = message[22:26]
                        self.interruptAddressSeqMap[address] = self.serverSequence
                        self.serverSequence += 1
                    elif "P1" in message:
                        address = message[12:16]
                        self.interruptAddressSeqMap[address] = self.serverSequence
                        self.serverSequence += 1
                    if self.serverSequence == 1000:
                        self.serverSequence = 1
                if "SCp" in msg:
                    log.debug(f"{self.deviceName} receives reboot request, disconnecting")
                    return
                time.sleep(0.1)
        except Exception as e:
            log.exception("Got exception: %s", str(e))
            time.sleep(5)

    def Destroy(self) -> None:
        self._shutdown.set()
        self.thread.join()
        log.info(f"Destroy [{self.deviceName}] AIOI server")

if __name__ == "__main__":
    server = DummyAIOIServer("test", 5003)