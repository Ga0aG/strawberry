// 实时状态更新客户端
export class RobotWebSocket {
    private socket: WebSocket | null = null;

    connect(robotId: string, onMessage: (status: RobotStatus) => void) {
      this.socket = new WebSocket(`ws://localhost:8000/ws/robot/${robotId}`);

      this.socket.onmessage = (event) => {
        const status: RobotStatus = JSON.parse(event.data);
        onMessage(status);
      };

      this.socket.onclose = () => {
        console.log("WebSocket disconnected");
      };
    }

    sendCommand(command: string) {
      this.socket?.send(JSON.stringify({ action: command }));
    }

    disconnect() {
      this.socket?.close();
    }
  }

  interface RobotStatus {
    id: string;
    position: { x: number; y: number };
    battery: number;
    isOnline: boolean;
  }