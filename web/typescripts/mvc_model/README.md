# 文件结构

├── server/                  # Python 后端 (FastAPI)
│   ├── models/             # 数据模型层
│   │   ├── robot.py        # 机器人状态模型
│   │   └── file.py         # 文件元数据模型
│   ├── controllers/        # 控制器层
│   │   ├── robot_ctl.py    # 机器人状态控制逻辑
│   │   └── file_ctl.py     # 文件上传下载逻辑
│   ├── routes/             # 路由层 (类似视图层)
│   │   ├── ws_routes.py    # WebSocket 路由
│   │   └── api_routes.py   # REST API 路由
│   └── main.py             # 入口文件
│
├── client/                 # TypeScript 前端 (React)
│   ├── src/
│   │   ├── services/       # 前端服务层
│   │   │   ├── ws.ts       # WebSocket 客户端
│   │   │   └── api.ts      # 文件上传下载 API
│   │   ├── components/     # React 组件
│   │   └── App.tsx         # 主界面
│   └── ...                 # 其他前端配置