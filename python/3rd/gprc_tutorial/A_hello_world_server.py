from concurrent import futures
import grpc
import A_hello_world_pb2
import A_hello_world_pb2_grpc

class CalServicer(A_hello_world_pb2_grpc.CalServicer):
  def Add(self, request, context):   # Add函数的实现逻辑
    print("Add function called")
    return A_hello_world_pb2.ResultReply(number=request.number1 + request.number2)

  def Multiply(self, request, context):   # Multiply函数的实现逻辑
    print("Multiply service called")
    return A_hello_world_pb2.ResultReply(number=request.number1 * request.number2)

def serve():
  server = grpc.server(futures.ThreadPoolExecutor(max_workers=5))
  A_hello_world_pb2_grpc.add_CalServicer_to_server(CalServicer(),server)
  server.add_insecure_port("[::]:50051")
  server.start()
  print("grpc server start...")
  server.wait_for_termination()

if __name__ == '__main__':
  serve()