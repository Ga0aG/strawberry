// import { v4 as uuidv4 } from 'uuid';
interface Greeting {
  text: string;
  display: () => void;
}

class HelloWorld implements Greeting {
  private downloadBtn: HTMLButtonElement;
  private getControlSourceBtn: HTMLButtonElement;
  private uploadBtn: HTMLButtonElement;
  constructor(public text: string) {
    // 创建按钮元素（如果未在HTML预定义）
    this.downloadBtn = document.createElement("button");
    this.downloadBtn.textContent = "下载日志文件";
    this.downloadBtn.className = "download-button";
    this.getControlSourceBtn = document.createElement("button");
    this.getControlSourceBtn.textContent = "获取控制源";
    this.getControlSourceBtn.className = "get-button";
    this.uploadBtn = document.createElement("button");
    this.uploadBtn.textContent = "上传文件按钮";
    this.uploadBtn.className = "upload-button";
  }

  display(): void {
    const el = document.getElementById("app")!;
    el.innerHTML = `<h1>${this.text}</h1>`;
    // 添加按钮到DOM
    el.appendChild(this.downloadBtn);
    el.appendChild(this.getControlSourceBtn);
    el.appendChild(this.uploadBtn);
    // 绑定点击事件（处理异步错误）
    this.downloadBtn.addEventListener("click", async () => {
      this.downloadBtn.disabled = true;
      try {
        const success = await this.download_file("app_logs.zip");
        if (!success) {
          alert("下载超时或文件不存在！");
        }
      } catch (err) {
        console.error("下载失败:", err);
        alert("下载过程中发生错误！");
      } finally {
        this.downloadBtn.disabled = false;
      }
    });
    this.getControlSourceBtn.addEventListener("click", async () => {
      this.getControlSourceBtn.disabled = true;
      try {
        const success = await this.get_control_source();
        if (!success) {
          alert("下载超时或文件不存在！");
        }
      } catch (err) {
        console.error("下载失败:", err);
        alert("下载过程中发生错误！");
      } finally {
        this.getControlSourceBtn.disabled = false;
      }
    });
    this.uploadBtn.addEventListener("click", () => {
      const fileInput = document.createElement("input");
      fileInput.type = "file";
      // fileInput.accept = '*/*'; // 可根据需求限制文件类型
      fileInput.onchange = async () => {
        const file = fileInput.files?.[0];
        if (file) {
          this.uploadBtn.disabled = true;
          try {
            await this.upload_file(file);
            // if (!success) {
            //     alert("文件上传失败！");
            // }
          } catch (err) {
            console.error("文件上传失败:", err);
          } finally {
            this.uploadBtn.disabled = false;
          }
        }
      };
      fileInput.click();
    });
  }
  public async get_control_source(): Promise<boolean> {
    // 从服务器日志来看，浏览器在发起 GET 请求之前，先发送了一个 OPTIONS 请求。这是因为浏览器在跨域请求时会进行 预检请求（Preflight Request），以确保服务器允许该跨域请求。
    const urlGet = new URL("http://0.0.0.0:8001/astribot/control_source");
    const response = await fetch(urlGet.toString(), {
      headers: {
        "Content-Type": "application/json",
      },
      method: "GET",
    });
    if (!response.ok) {
      console.error("Error downloading file:", response.statusText);
      return false;
    }
    console.log(response.body);
    return true;
  }
  public async download_file(str_file_name: string): Promise<boolean> {
    const urlGet = new URL("http://0.0.0.0:8001/download");
    const response = await fetch(urlGet.toString(), {
      headers: {
        "Content-Type": "application/json",
      },
      method: "GET",
    });
    if (!response.ok) {
      console.error("Error downloading file:", response.statusText);
      return false;
    }
    const reader = response.body?.getReader();
    if (!reader) {
      throw new Error("Failed to read stream");
    }

    const contentLength = +(response.headers.get("Content-Length") || 0);
    let receivedLength = 0;
    const chunks: Uint8Array[] = [];
    // const log_interval = setInterval(async () => {
    //     if (contentLength) {
    //         console.log(`Progress: ${((receivedLength / contentLength) * 100).toFixed(2)}%`);
    //     }
    // }, 1000);
    let isTimeout = false;
    setTimeout(() => {
      isTimeout = true;
      reader.cancel();
    }, 90 * 1000);
    while (!isTimeout) {
      const { done, value } = await reader.read();
      if (done) {
        break;
      }
      chunks.push(value);
      receivedLength += value.length;
      console.log(
        `Progress: ${((receivedLength / contentLength) * 100).toFixed(2)}%`
      );
    }
    // clearInterval(log_interval);
    if (isTimeout) {
      return false;
    }
    // 合并分块并触发下载
    const blob = new Blob(chunks);
    if (blob.size <= 500) {
      console.error("没有日志文件");
    }
    let url = window.URL.createObjectURL(blob);
    let a = document.createElement("a");
    a.style.display = "none";
    a.href = url;
    a.download = str_file_name;
    document.body.appendChild(a);
    a.click();
    window.URL.revokeObjectURL(url);
    return true;
  }
  public async read_file(str_type: string, data: Blob) {
    return new Promise<Blob>((resolve) => {
      let reader = new FileReader();
      reader.readAsArrayBuffer(data);
      reader.onload = async (e) => {
        let blob = new Blob([(e.target as any).result], { type: str_type });
        resolve(blob);
      };
      reader.onloadend = () => {
        reader.abort();
      };
    });
  }
  public async upload_file(
    file: File
    // fun_progress: Function | undefined = undefined,
    // n_chunk_size: number = FILE_CHUNK_SIZE
  ): Promise<void> {
    const str_task_id = "98776ae5-ac95-4bc4-9148-8ed73ec1c425"; // uuidv4();
    const n_chunk_size = 50 * 1024 * 1024;
    let n_total_chunk = Math.ceil(file.size / n_chunk_size);

    let n_progress = 0;
    for (
      let n_chunk_index = 0;
      n_chunk_index < n_total_chunk;
      n_chunk_index++
    ) {
      console.log("upload_file", `start read blob: ${n_chunk_index + 1}`);
      let blob: Blob | null = await this.read_file(
        file.type,
        file.slice(
          n_chunk_index * n_chunk_size,
          (n_chunk_index + 1) * n_chunk_size
        )
      );
      console.log("upload_file", `start upload blob:${n_total_chunk}`);
      await this.upload_chunk(
        str_task_id,
        blob,
        n_chunk_index,
        file,
        n_total_chunk
      );
      console.log("upload_file", `finish uploading chunk ${n_chunk_index + 1}`);
      console.log("progress: ", (n_chunk_index + 1 / n_total_chunk) * 100);
      // blob = null;
      URL.revokeObjectURL(blob as any);
    }
  }
  private upload_chunk(
    str_task_id: string,
    chunk: Blob,
    index: number,
    file: File,
    n_chunk_num: number
  ) {
    const url = new URL("http://0.0.0.0:8001/upload_chunk");
    return new Promise<void>((resolve) => {
      const form_data = new FormData();
      // form_data.append("client_id", AstribotSDKApp.get_app().get_client_id());
      form_data.append("file", chunk);
      form_data.append("task_id", str_task_id);
      form_data.append("fileName", "upload.zip");
      form_data.append("chunkIndex", index.toString());
      form_data.append("totalChunks", n_chunk_num.toString());

      console.warn(`上传文件分块: ${index}/${n_chunk_num}`);

      const xhr = new XMLHttpRequest();
      xhr.open("POST", url.toString(), true);
      xhr.onload = () => {
        if (xhr.status === 200) {
          resolve();
        }
      };
      xhr.send(form_data);
    });
  }
}

// new HelloWorld("TypeScript!").display();
window.addEventListener("DOMContentLoaded", () => {
  const hello = new HelloWorld("欢迎使用日志下载服务");
  hello.display();
});
