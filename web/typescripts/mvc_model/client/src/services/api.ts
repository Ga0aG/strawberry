// 文件上传下载接口
import axios, { AxiosProgressEvent } from 'axios';

const API_BASE = 'http://localhost:8000/api';

export const uploadLargeFile = async (
    formData: FormData,
    onProgress?: (progress: number) => void
) => {
    return axios.post(`${API_BASE}/upload`, formData, {
        headers: { 'Content-Type': 'multipart/form-data' },
        onUploadProgress: (progressEvent: AxiosProgressEvent) => {
            if (progressEvent.total) {
                const percent = Math.round(
                    (progressEvent.loaded * 100) / progressEvent.total
                );
                onProgress?.(percent);
            }
        }
    });
};

export const downloadFile = async (filename: string) => {
    const response = await axios.get(`${API_BASE}/download/${filename}`, {
        responseType: 'blob'
    });
    const url = window.URL.createObjectURL(new Blob([response.data]));
    const link = document.createElement('a');
    link.href = url;
    link.setAttribute('download', filename);
    document.body.appendChild(link);
    link.click();
    link.remove();
};