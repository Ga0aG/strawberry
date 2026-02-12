import { useState, useEffect } from 'react';
import { RobotWebSocket } from './services/ws';
import { FileUpload } from './components/FileUpload';
import { downloadFile } from './services/api';

interface RobotStatus {
    id: string;
    position: { x: number; y: number };
    battery: number;
    isOnline: boolean;
}

export default function App() {
    const [status, setStatus] = useState<RobotStatus>({
        id: 'robot-001',
        position: { x: 0, y: 0 },
        battery: 100,
        isOnline: false
    });
    const [wsClient] = useState(new RobotWebSocket());

    // 初始化 WebSocket 连接
    useEffect(() => {
        wsClient.connect(status.id, (newStatus) => {
            setStatus(prev => ({ ...prev, ...newStatus }));
        });

        return () => wsClient.disconnect();
    }, []);

    return (
        <div className="container">
            <h1>机器人监控系统</h1>

            {/* 状态显示 */}
            <div className="status-panel">
                <p>在线状态: {status.isOnline ? '🟢 在线' : '🔴 离线'}</p>
                <p>电量: {status.battery}%</p>
                <p>位置: ({status.position.x.toFixed(2)}, {status.position.y.toFixed(2)})</p>
            </div>

            {/* 文件上传 */}
            <FileUpload />

            {/* 文件下载示例 */}
            <button onClick={() => downloadFile('example.zip')}>
                下载示例文件
            </button>
        </div>
    );
}