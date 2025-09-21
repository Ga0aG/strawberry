import sys
from funasr import AutoModel
from funasr.utils.postprocess_utils import rich_transcription_postprocess

# model_dir = "iic/SenseVoiceSmall"
model_dir = "/home/ga/.cache/modelscope/hub/models/iic/SenseVoiceSmall"
# model_dir = "paraformer-zh-streaming"
# model_dir = "/home/ga/.cache/modelscope/hub/models/iic/speech_paraformer-large_asr_nat-zh-cn-16k-common-vocab8404-online"
# vad_model = "fsmn-vad"
vad_model = "/home/ga/.cache/modelscope/hub/models/iic/speech_fsmn_vad_zh-cn-16k-common-pytorch"

model = AutoModel(model=model_dir,
                  model_revision="v2.0.4",
                  disable_update=True)

# model = AutoModel(
#     model=model_dir,  # The name of the model, or the path to the model on the local disk. 模型名称，或本地磁盘中的模型路径。
#     vad_model=vad_model,  #  This indicates the activation of VAD (Voice Activity Detection). The purpose of VAD is to split long audio into shorter clips. In this case, the inference time includes both VAD and SenseVoice total consumption, and represents the end-to-end latency. If you wish to test the SenseVoice model's inference time separately, the VAD model can be disabled. 表示开启VAD，VAD的作用是将长音频切割成短音频，此时推理耗时包括了VAD与SenseVoice总耗时，为链路耗时，如果需要单独测试SenseVoice模型耗时，可以关闭VAD模型。
#     vad_kwargs={
#         "max_single_segment_time": 30000
#     },  # 表示VAD模型配置,max_single_segment_time: 表示vad_model最大切割音频时长, 单位是毫秒ms。
#     device="cuda:0",
#     disable_update=True,
# )

# en
res = model.generate(
    # input="/home/ga/Downloads/input.wav",
    input="/home/ga/workspace/strawberry/machine_learning/funasr_/my_recording.wav",
    # input=f"{model.model_path}/example/en.mp3",
    cache={},
    language="auto",  # "zn", "en", "yue", "ja", "ko", "nospeech"
    use_itn=True,  # Whether the output result includes punctuation and inverse text normalization. 输出结果中是否包含标点与逆文本正则化。
    batch_size_s=60,  # 表示采用动态batch，batch中总音频时长，单位为秒s。
    merge_vad=True,  # 是否将 vad 模型切割的短音频碎片合成，合并后长度为merge_length_s，单位为秒s。
    merge_length_s=15,
    ban_emo_unk=True,
)
text = rich_transcription_postprocess(res[0]["text"])
print(text)
