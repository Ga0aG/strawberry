# 机器人状态控制器 (业务逻辑)
from fastapi import WebSocket
from models.robot import RobotStatus
import json
from typing import Dict

class RobotController:
    def __init__(self):
        self.active_connections: Dict[str, WebSocket] = {}  # 维护在线连接

    async def broadcast_status(self, robot_id: str, status: RobotStatus):
        """广播机器人状态给所有订阅者"""
        if robot_id in self.active_connections:
            await self.active_connections[robot_id].send_text(
                status.model_dump_json()
            )

    async def websocket_endpoint(self, websocket: WebSocket, robot_id: str):
        """处理 WebSocket 连接"""
        await websocket.accept()
        self.active_connections[robot_id] = websocket
        try:
            while True:
                # 接收前端控制指令（可选）
                data = await websocket.receive_text()
                # 处理指令逻辑...
        except Exception as e:
            del self.active_connections[robot_id]