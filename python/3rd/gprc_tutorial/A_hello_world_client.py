import A_hello_world_pb2
import A_hello_world_pb2_grpc
import grpc

def run(n, m):
  channel = grpc.insecure_channel('localhost:50051') # 连接上gRPC服务端
  stub = A_hello_world_pb2_grpc.CalStub(channel)
  response = stub.Add(A_hello_world_pb2.AddRequest(number1=n, number2=m))  # 执行计算命令
  print(f"{n} + {m} = {response.number}")
  response = stub.Multiply(A_hello_world_pb2.MultiplyRequest(number1=n, number2=m))
  print(f"{n} * {m} = {response.number}")

if __name__ == "__main__":
  run(100, 300)
