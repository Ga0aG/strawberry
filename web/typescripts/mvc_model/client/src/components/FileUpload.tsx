import { useState } from 'react';
import { uploadLargeFile } from '../services/api';

export const FileUpload = () => {
  const [progress, setProgress] = useState(0);

  const handleUpload = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;

    // 分块上传示例
    const chunkSize = 50 * 1024 * 1024; // 50MB
    const totalChunks = Math.ceil(file.size / chunkSize);

    for (let i = 0; i < totalChunks; i++) {
      const chunk = file.slice(i * chunkSize, (i + 1) * chunkSize);
      const formData = new FormData();
      formData.append('file', chunk);
      formData.append('chunkIndex', i.toString());
      formData.append('totalChunks', totalChunks.toString());

      await uploadLargeFile(formData);
      setProgress(Math.round(((i + 1) / totalChunks) * 100));
    }
  };

  return (
    <div>
      <input type="file" onChange={handleUpload} />
      {progress > 0 && <progress value={progress} max="100" />}
    </div>
  );
};