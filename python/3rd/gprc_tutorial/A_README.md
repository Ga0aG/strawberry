Reference: https://zhuanlan.zhihu.com/p/97893141

1. Use grpc_tools to generate python code

   ```python3 -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. ./A_hello_world.proto```

2. Open terminal run `python3 A_hello_world_server.py`

3. Open another termianl run `python3 A_hello_world_client.py`