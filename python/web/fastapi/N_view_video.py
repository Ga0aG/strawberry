from fastapi import FastAPI, HTTPException, Query
from fastapi.responses import FileResponse, HTMLResponse
from fastapi.middleware.cors import CORSMiddleware
import mimetypes
import os
import uvicorn
from urllib.parse import quote

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
    expose_headers=[
        "file-size",
        "file-md5",
    ],  # 浏览器默认只会暴露部分安全的 HTTP 头字段，需要在服务端设置
)


VIDEO_FOLDER = "/mnt/training_data"

@app.get("/video")
async def serve_video(path: str = Query(..., description="Absolute file path to the video")):
    abs_path = os.path.join(VIDEO_FOLDER, path)
    if not os.path.isfile(abs_path):
        raise HTTPException(status_code=404, detail="File not found")

    media_type, _ = mimetypes.guess_type(path)
    if media_type is None:
        media_type = "application/octet-stream"

    filename = os.path.basename(abs_path)
    return FileResponse(path=abs_path, media_type=media_type, filename=filename)


@app.get("/watch", response_class=HTMLResponse)
async def watch(path: str = Query(..., description="Absolute file path to the video")):
    src = f"/video?path={quote(path)}"
    html = (
        "<!doctype html>\n"
        "<html><head><meta charset=\"utf-8\"><title>Watch</title></head>"
        "<body style=\"margin:0;display:flex;align-items:center;justify-content:center;height:100vh;background:#111;\">"
        f"<video src=\"{src}\" controls style=\"max-width:100%;max-height:100%;background:#000\"></video>"
        "</body></html>"
    )
    return HTMLResponse(content=html)

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
