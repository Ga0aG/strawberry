# 大文件传输控制器
from fastapi import UploadFile, HTTPException
from fastapi.responses import StreamingResponse
import aiofiles
import os

class FileController:
    CHUNK_SIZE = 1024 * 1024 * 50  # 50MB 分块

    async def upload_large_file(self, file: UploadFile):
        """流式上传大文件"""
        file_path = f"uploads/{file.filename}"
        async with aiofiles.open(file_path, "wb") as f:
            while chunk := await file.read(self.CHUNK_SIZE):
                await f.write(chunk)
        return {"filename": file.filename}

    async def download_large_file(self, filename: str):
        """流式下载大文件"""
        file_path = f"uploads/{filename}"
        if not os.path.exists(file_path):
            raise HTTPException(status_code=404, detail="File not found")

        async def file_stream():
            async with aiofiles.open(file_path, "rb") as f:
                while chunk := await f.read(self.CHUNK_SIZE):
                    yield chunk

        return StreamingResponse(
            file_stream(),
            media_type="application/octet-stream",
            headers={"Content-Disposition": f"attachment; filename={filename}"}
        )