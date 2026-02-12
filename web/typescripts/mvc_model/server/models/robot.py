# 机器人状态模型 (数据层)
from pydantic import BaseModel
from typing import Dict

class RobotStatus(BaseModel):
    id: str
    position: Dict[str, float]  # 坐标位置
    battery: float              # 电量百分比
    is_online: bool = False