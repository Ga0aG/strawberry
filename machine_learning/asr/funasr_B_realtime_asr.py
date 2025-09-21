import threading
import queue
import time
import numpy as np
import pyaudio
import webrtcvad
from funasr import AutoModel


class RealTimeASR:
    def __init__(self, model_size="small"):
        """
        初始化实时语音识别系统

        参数:
            model_size: 模型大小，可选 "small", "medium", "large"
        """
        # 初始化FunASR模型
        print("正在加载FunASR模型...")
        if model_size == "small":
            model_dir = "/home/ga/.cache/modelscope/hub/models/iic/SenseVoiceSmall"
        elif model_size == "medium":
            model_dir = "paraformer-zh-streaming"
        else:
            model_dir = "paraformer-zh-streaming"
        self.model = AutoModel(
            model=model_dir, model_revision="v2.0.4", disable_update=True
        )
        print("模型加载完成")

        # 音频参数
        self.sample_rate = 48000
        self.chunk_size = 960  # 30ms chunks for VAD
        self.format = pyaudio.paInt16
        self.channels = 1

        # 初始化VAD（语音活动检测）
        self.vad = webrtcvad.Vad(2)  # 中等灵敏度

        # 音频队列
        self.audio_queue = queue.Queue()
        self.is_recording = False

        # 初始化PyAudio
        self.audio = pyaudio.PyAudio()

    def audio_callback(self, in_data, frame_count, time_info, status):
        """PyAudio回调函数，用于捕获音频数据"""
        if self.is_recording:
            # 将音频数据放入队列
            self.audio_queue.put(in_data)
        return (None, pyaudio.paContinue)

    def record_audio(self):
        """录制音频"""
        print("开始录制音频...")
        self.is_recording = True

        # 打开音频流
        stream = self.audio.open(
            format=self.format,
            channels=self.channels,
            rate=self.sample_rate,
            input=True,
            frames_per_buffer=self.chunk_size,
            stream_callback=self.audio_callback,
        )

        stream.start_stream()

        try:
            while self.is_recording:
                time.sleep(0.1)
        except KeyboardInterrupt:
            self.is_recording = False

        stream.stop_stream()
        stream.close()

    def process_audio(self):
        """处理音频数据并进行语音识别"""
        audio_buffer = b""
        silence_frames = 0
        max_silence_frames = 20  # 最多允许20帧静音

        print("开始处理音频...")

        while self.is_recording or not self.audio_queue.empty():
            try:
                # 从队列中获取音频数据
                data = self.audio_queue.get(timeout=1)
                audio_buffer += data

                # 使用VAD检测是否有语音活动
                is_speech = self.vad.is_speech(data, self.sample_rate)

                if is_speech:
                    silence_frames = 0
                    # 如果有语音活动，继续收集数据
                    if len(audio_buffer) > self.sample_rate * 2:  # 最多收集2秒音频
                        self.transcribe_audio(audio_buffer)
                        audio_buffer = b""
                else:
                    silence_frames += 1
                    # 如果连续静音帧超过阈值，转录当前缓冲区
                    if silence_frames >= max_silence_frames and len(audio_buffer) > 0:
                        self.transcribe_audio(audio_buffer)
                        audio_buffer = b""
                        silence_frames = 0

            except queue.Empty:
                continue

        # 转录剩余的音频
        if len(audio_buffer) > 0:
            self.transcribe_audio(audio_buffer)

    def transcribe_audio(self, audio_data):
        """使用FunASR转录音频"""
        try:
            # 将字节数据转换为numpy数组
            audio_np = (
                np.frombuffer(audio_data, dtype=np.int16).astype(np.float32) / 32768.0
            )

            # 使用FunASR进行语音识别
            result = self.model.generate(
                input=audio_np, cache={}, is_final=True, chunk_size=[0, 10, 5]
            )

            if result and len(result) > 0 and "text" in result[0]:
                text = result[0]["text"]
                if text.strip():
                    print(f"识别结果: {text}")
        except Exception as e:
            print(f"转录错误: {e}")

    def start(self):
        """启动实时语音识别"""
        print("启动实时语音识别系统")
        print("按Ctrl+C停止录制")

        # 创建录制线程
        record_thread = threading.Thread(target=self.record_audio)
        record_thread.daemon = True
        record_thread.start()

        # 创建处理线程
        process_thread = threading.Thread(target=self.process_audio)
        process_thread.daemon = True
        process_thread.start()

        try:
            # 等待线程结束
            record_thread.join()
            process_thread.join()
        except KeyboardInterrupt:
            print("\n正在停止录制...")
            self.is_recording = False

    def cleanup(self):
        """清理资源"""
        self.audio.terminate()


# 主函数
if __name__ == "__main__":
    asr_system = RealTimeASR(model_size="small")

    try:
        asr_system.start()
    except Exception as e:
        print(f"发生错误: {e}")
    finally:
        asr_system.cleanup()
