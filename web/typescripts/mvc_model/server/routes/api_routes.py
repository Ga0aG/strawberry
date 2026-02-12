from fastapi import APIRouter, UploadFile
from controllers.file_ctl import FileController
from fastapi.responses import StreamingResponse

router = APIRouter()
file_ctl = FileController()

@router.post("/upload")
async def upload_file(file: UploadFile):
    return await file_ctl.upload_large_file(file)

@router.get("/download/{filename}")
async def download_file(filename: str):
    return await file_ctl.download_large_file(filename)
