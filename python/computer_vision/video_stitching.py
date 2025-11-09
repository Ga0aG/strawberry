# python3.10

import sys
import cv2
import skvideo.io
import numpy as np
from IPython import embed

def stitching(cap1, cap2, width, fixedsize, N):
    def getTransitionMatrix(train, query):
        # 创建SURF特征检测器
        # surf = cv2.xfeatures2d.SURF_create()
        surf = cv2.SIFT_create()

        # 检测关键点和计算描述符
        kp1, des1 = surf.detectAndCompute(query, None)
        kp2, des2 = surf.detectAndCompute(train, None)

        # 创建暴力匹配器
        bf = cv2.BFMatcher()
        matches = bf.knnMatch(des1, des2, k=2)

        # 应用Lowe's ratio test筛选好的匹配点
        good = []
        for m, n in matches:
            if m.distance < 0.75 * n.distance:
                good.append(m)

        if len(good) >= 4:  # 至少需要4个点来计算单应性矩阵
            src = np.float32([kp1[m.queryIdx].pt for m in good]).reshape(-1, 1, 2)
            dst = np.float32([kp2[m.trainIdx].pt for m in good]).reshape(-1, 1, 2)

            # 计算单应性矩阵
            M, mask = cv2.findHomography(src, dst, cv2.RANSAC, 5.0)
            return M
        else:
            # 如果没有足够匹配点，返回单位矩阵
            return np.eye(3)

    def fusion(train, query, M):
        # 将query图像根据变换矩阵M进行透视变换
        dst = cv2.warpPerspective(query, M, fixedsize)

        # 创建结果图像，将train放在左侧
        result = dst.copy()
        result[0:train.shape[0], 0:train.shape[1]] = train

        # 在拼接边界处进行融合，减少接缝
        crack = train.shape[1] - 1 - int(width/2)
        for i in range(-int(width/2), int(width/2) + 1):
            alpha = (width/2 - abs(i)) / width  # 混合权重
            result[0:train.shape[0], crack + i, :] = (
                alpha * train[:, crack + i, :] +
                (1 - alpha) * dst[0:train.shape[0], crack + i, :]
            )
        return result

    def smooth(lst, W):  # W是奇数
        """对列表进行平滑滤波"""
        weights = np.hamming(W)  # 汉明窗
        # 卷积平滑
        smoothed = np.convolve(lst, weights/weights.sum(), mode='valid')

        # 处理边界，保持长度不变
        half_w = W // 2
        lst2 = lst[:half_w].copy() #.tolist()
        lst2.extend(smoothed.tolist())
        lst2.extend(lst[-half_w:]) # .tolist())

        return np.array(lst2)

    # 主处理流程
    count = min(cap1.shape[0], cap2.shape[0])  # 取两个视频的最小帧数
    frames = []
    Ms = []

    # 第一步：计算每一帧的变换矩阵
    for i in range(count):
        M = getTransitionMatrix(cap1[i], cap2[i])
        Ms.append(M)

    Ms = np.array(Ms)

    # 第二步：对变换矩阵进行时序平滑
    smoothed_Ms = Ms.copy()
    for i in range(Ms.shape[1]):  # 遍历矩阵的行
        for j in range(Ms.shape[2]):  # 遍历矩阵的列
            # 提取该位置在所有帧中的值序列
            values = Ms[:, i, j].tolist()
            # 应用平滑
            smoothed_values = smooth(values, N)
            smoothed_Ms[:, i, j] = smoothed_values

    # 第三步：使用平滑后的变换矩阵进行图像融合
    for i, M in enumerate(smoothed_Ms):
        frame = fusion(cap1[i], cap2[i], M)
        frames.append(frame)

    return frames

def read_video_frames(video_path):
    """
    读取视频并返回帧的numpy数组
    """
    cap = cv2.VideoCapture(video_path)

    if not cap.isOpened():
        raise ValueError(f"无法打开视频文件: {video_path}")

    frames = []
    frame_count = 0

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # 转换为RGB格式 (OpenCV默认是BGR)
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        frames.append(frame_rgb)
        frame_count += 1

        # 可选：显示进度
        if frame_count % 100 == 0:
            print(f"已读取 {frame_count} 帧...")

    cap.release()
    print(f"视频读取完成，共 {frame_count} 帧")

    return np.array(frames)

def save_video_advanced(frames, output_path, fps=30, quality=90):
    """
    增强版的视频保存函数，支持质量设置

    参数:
        frames: 帧序列
        output_path: 输出路径
        fps: 帧率
        quality: 视频质量 (0-100)
    """
    if len(frames) == 0:
        print("错误：没有帧可保存")
        return

    height, width = frames[0].shape[0], frames[0].shape[1]

    # 根据文件扩展名选择编码器
    file_ext = output_path.lower().split('.')[-1]

    if file_ext == 'mp4':
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    elif file_ext == 'avi':
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
    else:
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # 默认使用mp4v

    # 创建VideoWriter
    out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))

    if not out.isOpened():
        print(f"错误：无法创建视频文件 {output_path}")
        return

    print(f"开始保存视频: {output_path}")
    print(f"尺寸: {width}x{height}, 帧率: {fps}, 总帧数: {len(frames)}")

    for i, frame in enumerate(frames):
        # 确保帧的数据类型是uint8
        if frame.dtype != np.uint8:
            frame = np.uint8(np.clip(frame, 0, 255))

        # RGB转BGR
        frame_bgr = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
        out.write(frame_bgr)

        # 进度显示
        if i % 50 == 0 or i == len(frames) - 1:
            progress = (i + 1) / len(frames) * 100
            print(f"进度: {i+1}/{len(frames)} 帧 ({progress:.1f}%)")

    out.release()
    print(f"视频保存完成: {output_path}")

# 使用方法：
# save_video_advanced(result_frames, "my_stitched_video.mp4", fps=25, quality=90)
# save_video_advanced(result_frames, "output.avi", fps=30, quality=80)

# 使用示例
"""
假设cap1和cap2是已经读取的视频帧数组
fixedsize = (width, height)  # 输出全景图的固定尺寸
width = 20  # 融合区域的宽度
N = 5  # 平滑窗口大小

result_frames = stitching(cap1, cap2, width, fixedsize, N)
"""
# cap1 = skvideo.io.vread(sys.argv[1])
# cap2 = skvideo.io.vread(sys.argv[2])
# cap1 = cv2.VideoCapture(sys.argv[1])
# cap2 = cv2.VideoCapture(sys.argv[2])
cap1 = read_video_frames(sys.argv[1])
cap2 = read_video_frames(sys.argv[2])
# embed()
fixedsize = (900, 1280)  # 输出全景图的固定尺寸
width = 20  # 融合区域的宽度
N = 5  # 平滑窗口大小

result_frames = stitching(cap1, cap2, width, fixedsize, N)
save_video_advanced(result_frames, "output.mp4")