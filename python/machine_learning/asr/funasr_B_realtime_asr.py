import numpy as np
import pyaudio
import queue
import threading
import time
import torch
import torchaudio
import traceback
import webrtcvad
from funasr import AutoModel
from funasr.utils.postprocess_utils import rich_transcription_postprocess

from utils.logger import create_logger

logger = create_logger(__name__)


class RealTimeASR:
    def __init__(self, model_size="small"):
        """
        初始化实时语音识别系统

        参数:
            model_size: 模型大小，可选 "small", "medium", "large"
        """
        # 初始化FunASR模型
        logger.info("正在加载模型...")
        start_time = time.time()
        if model_size == "small":
            model_dir = "/home/ga/.cache/modelscope/hub/models/iic/SenseVoiceSmall"
        elif model_size == "medium":
            model_dir = "paraformer-zh-streaming"
        else:
            model_dir = "paraformer-zh-streaming"
        self.model = AutoModel(
            model=model_dir,
            model_revision="v2.0.4",
            vad_kwargs={
                "max_single_segment_time": 30000
            },  # 表示VAD模型配置,max_single_segment_time: 表示vad_model最大切割音频时长, 单位是毫秒ms。
            device="cuda:0",
            disable_update=True,
        )

        # 音频参数
        self.sample_rate = 16000  # 　据说这是Funasr的默认采样率
        """
        chunk_size在FunASR中是一个流式推理的重要参数，通常是一个包含3个元素的数组[前向chunk, 后向chunk, 编码器chunk]
        - 第一个元素（前向chunk）：解码器的前向chunk大小，控制流式识别的延迟
        - 第二个元素（后向chunk）：解码器的后向chunk大小，影响识别准确率
        - 第三个元素（编码器chunk）：编码器的chunk大小，控制计算效率
        """
        self.chunk_size = [5, 10, 5]
        self.format = pyaudio.paInt16  # 16位 = 2字节/样本
        self.channels = 1
        chunk_interval = 10
        self.CHUNK = 512# int(self.sample_rate / 1000 * 60)  # 60ms

        # 初始化VAD（语音活动检测）
        vad_model, vad_utils = torch.hub.load(
            # repo_or_dir='snakers4/silero-vad',
            # source='github',
            repo_or_dir='/home/ga/.cache/torch/hub/snakers4_silero-vad_master',
            source='local',
            model='silero_vad',
            force_reload=False
        )
        (self.get_speech_timestamps, _, self.read_audio,
         self.VADIterator, self.collect_chunks) = vad_utils
        # 创建VAD迭代器（用于流式处理）
        self.vad_iterator = self.VADIterator(vad_model)
        self.hear_something_timestamp = None

        # 音频队列
        self.audio_queue = queue.Queue()
        self.is_recording = False

        # 初始化PyAudio
        self.audio = pyaudio.PyAudio()
        self.stream = self.audio.open(
            format=self.format,
            channels=self.channels,
            rate=self.sample_rate,
            input=True,
            frames_per_buffer=self.CHUNK,
        )
        logger.info(f"模型加载完成: {time.time()-start_time}")

    def record_audio(self):
        """录制音频"""
        logger.info("开始录制音频...")
        self.is_recording = True

        # 打开音频流
        self.stream.start_stream()

        try:
            while self.is_recording:
                time.sleep(0.1)
        except KeyboardInterrupt:
            self.is_recording = False

        self.stream.stop_stream()
        self.stream.close()

    def process_audio(self):
        """处理音频数据并进行语音识别"""
        audio_buffer = b""
        speech_frames = 0
        silence_frames = 0
        min_speech_frames = 10  # 最少需要语音帧数才开始识别
        max_silence_frames = 20  # 静音超过这个阈值认为说话结束
        max_audio_duration = 10.0  # 最大音频长度（秒）

        logger.info("开始处理音频...")

        while self.is_recording:  # or not self.audio_queue.empty():
            try:
                # 从队列中获取音频数据
                # data = self.audio_queue.get(timeout=1)
                data = self.stream.read(self.CHUNK, exception_on_overflow=False)
                if not data:
                    time.sleep(0.1)
                    continue
                is_speech = self.check_is_speech(data)
                if is_speech:
                    audio_buffer += data
                    speech_frames += 1
                    silence_frames = 0
                    # logger.debug(f"检测到语音，连续语音帧: {speech_frames}")
                else:
                    silence_frames += 1
                    # logger.debug(f"静音帧: {silence_frames}")

                # 计算当前音频时长
                current_duration = (
                    len(audio_buffer) / 2 / self.sample_rate
                )  # 16位=2字节

                # 判断是否应该转录
                should_transcribe = False

                # 情况1：检测到语音开始后又出现足够长的静音（一句话结束）
                if (
                    speech_frames >= min_speech_frames
                    and silence_frames >= max_silence_frames
                ):
                    should_transcribe = True
                    logger.info("检测到语句结束，开始转录")

                # 情况2：音频过长，强制转录（避免内存溢出和延迟过大）
                elif current_duration >= max_audio_duration:
                    should_transcribe = True
                    logger.info("音频过长，强制转录")

                if should_transcribe:
                    self.transcribe_audio(audio_buffer)
                    # 保留最后0.5秒音频作为上下文衔接
                    keep_duration = 0.5  # 保留0.5秒
                    keep_bytes = int(keep_duration * self.sample_rate * 2)
                    audio_buffer = (
                        audio_buffer[-keep_bytes:]
                        if len(audio_buffer) > keep_bytes
                        else b""
                    )
                    speech_frames = 0
                    silence_frames = 0

            except queue.Empty:
                continue
            except Exception as e:
                logger.info(f"Error {traceback.format_exc()}")

        # 转录剩余的音频
        if len(audio_buffer) > 0:
            self.transcribe_audio(audio_buffer)

    def check_is_speech(self, audio_data):
        """处理音频流"""
        # 转换为torch tensor
        if isinstance(audio_data, np.ndarray):
            audio_tensor = torch.from_numpy(audio_data).float()
        else:
            # 如果是bytes，先转换为numpy
            audio_np = np.frombuffer(audio_data, dtype=np.int16).astype(np.float32) / 32768.0
            audio_tensor = torch.from_numpy(audio_np).float()

        # 使用VAD迭代器
        # raise ValueError(f"Provided number of samples is {x.shape[-1]} (Supported values: 256 for 8000 sample rate, 512 for 16000)")
        speech_dict = self.vad_iterator(audio_tensor, return_seconds=True)

        if speech_dict:
            if speech_dict.get("start", None) is not None:
                self.hear_something_timestamp = speech_dict["start"]
                logger.debug(f"听见有人说话")
            if speech_dict.get("end", None) is not None:
                if self.hear_something_timestamp:
                    logger.debug(f"说话时长: {speech_dict['end'] - self.hear_something_timestamp}")
                    self.hear_something_timestamp = None
            return self.hear_something_timestamp is not None
        return self.hear_something_timestamp

    def transcribe_audio(self, audio_data):
        """使用FunASR转录音频"""
        try:
            # 使用FunASR进行语音识别
            result = self.model.generate(
                input=audio_data,
                cache={},
                language="zn",  # "zn", "en", "yue", "ja", "ko", "nospeech"
                use_itn=True,  # Whether the output result includes punctuation and inverse text normalization. 输出结果中是否包含标点与逆文本正则化。
                batch_size_s=60,  # 表示采用动态batch，batch中总音频时长，单位为秒s。
                ban_emo_unk=True,
                merge_vad=True,  # 是否将 vad 模型切割的短音频碎片合成，合并后长度为merge_length_s，单位为秒s。
                merge_length_s=15,
                chunk_size=[0, 10, 5],
            )
            logger.info(
                f"识别结果: {rich_transcription_postprocess(result[0]['text'])}"
            )
        except Exception as e:
            logger.info(f"转录错误: {e}")

    def start(self):
        """启动实时语音识别"""
        logger.info("启动实时语音识别系统")
        logger.info("按Ctrl+C停止录制")

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
            logger.info("\n正在停止录制...")
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
        logger.info(f"发生错误: {e}")
    finally:
        asr_system.cleanup()
