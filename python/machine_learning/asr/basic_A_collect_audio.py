import pyaudio
import wave
import threading
import time
import numpy

"""
>>> audio = pyaudio.PyAudio()
>>> audio.get_default_input_device_info()
{'index': 15, 'structVersion': 2, 'name': 'default', 'hostApi': 0, 'maxInputChannels': 128, 'maxOutputChannels': 128, 'defaultLowInputLatency': 0.021333333333333333, 'defaultLowOutputLatency': 0.021333333333333333, 'defaultHighInputLatency': 0.021333333333333333, 'defaultHighOutputLatency': 0.021333333333333333, 'defaultSampleRate': 48000.0}
"""


class AudioRecorder:
    def __init__(
        self,
        output_file="output.wav",
        record_seconds=10,
        channels=1,
        rate=16000,
        chunk=2**12,
    ):
        """
        初始化音频录制器

        参数:
            output_file: 输出文件名
            record_seconds: 录制时长（秒）
            channels: 声道数（1=单声道，2=立体声）
            rate: 采样率（Hz）, 可以选择defaultSampleRate
            chunk: 每次读取的帧数
        """
        self.output_file = output_file
        self.record_seconds = record_seconds
        self.channels = channels
        self.rate = rate
        self.chunk = chunk
        self.frames = []
        self.is_recording = False
        self.audio = pyaudio.PyAudio()

    def start_recording(self):
        """开始录制音频"""
        print("开始录制音频...")
        self.is_recording = True
        self.frames = []

        # 打开音频流
        self.stream = self.audio.open(
            format=pyaudio.paInt16,
            channels=self.channels,
            rate=self.rate,
            input=True,
            frames_per_buffer=self.chunk,
        )

        # 创建并启动录制线程
        self.recording_thread = threading.Thread(target=self._record)
        self.recording_thread.start()

    def _record(self):
        """录制音频的内部方法"""
        for _ in range(0, int(self.rate / self.chunk * self.record_seconds)):
            if not self.is_recording:
                break
            data = self.stream.read(self.chunk)
            self.frames.append(data)

        self.stop_recording()

    def stop_recording(self):
        """停止录制并保存文件"""
        if not self.is_recording:
            return

        print("停止录制...")
        self.is_recording = False

        # 停止并关闭流
        self.stream.stop_stream()
        self.stream.close()

        # 保存为WAV文件
        self.save_to_wav()

        print(f"音频已保存到: {self.output_file}")

    def save_to_wav(self):
        """将录制的音频保存为WAV文件"""
        wf = wave.open(self.output_file, "wb")
        wf.setnchannels(self.channels)
        wf.setsampwidth(self.audio.get_sample_size(pyaudio.paInt16))
        wf.setframerate(self.rate)
        wf.writeframes(b"".join(self.frames))
        wf.close()

    def cleanup(self):
        """清理资源"""
        self.audio.terminate()


# 使用示例
if __name__ == "__main__":
    # 创建录音器实例
    recorder = AudioRecorder(
        output_file="my_recording.wav",
        record_seconds=5,  # 录制5秒
        channels=1,
        rate=44100,  # CD质量的采样率
    )

    try:
        # 开始录制
        recorder.start_recording()

        # 等待录制完成
        print("录制中...按Ctrl+C提前停止")
        time.sleep(recorder.record_seconds)

    except KeyboardInterrupt:
        print("\n用户中断录制")
        recorder.stop_recording()
    finally:
        recorder.cleanup()
