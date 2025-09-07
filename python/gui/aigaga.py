import whisper
# import opencc

# ASR, Automatic Speech Recognition
model = whisper.load_model("base")
result = model.transcribe(audio="/home/ga/Recordings/20250315")
# cc = opencc.OpenCC()