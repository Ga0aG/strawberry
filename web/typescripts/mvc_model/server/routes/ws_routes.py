# WebSocket 路由
from fastapi import APIRouter, WebSocket
from controllers.robot_ctl import RobotController

router = APIRouter()
robot_ctl = RobotController()

@router.websocket("/ws/robot/{robot_id}")
async def websocket_route(websocket: WebSocket, robot_id: str):
    await robot_ctl.websocket_endpoint(websocket, robot_id)