import rclpy
from lifecycle_msgs.srv import ChangeState
from rclpy.lifecycle import LifecycleNode, LifecycleState, TransitionCallbackReturn
from rclpy.timer import Timer
from rclpy.executors import MultiThreadedExecutor
from std_msgs.msg import String
from example_interfaces.srv import AddTwoInts

class MyLifecycleNode(LifecycleNode):
    def __init__(self):
        super().__init__('listener_node')

        # 在CONFIGURE阶段创建实体（推荐）
        self.pub = None
        self.sub = None
        self.srv = None
        self.timer = None

    # 生命周期状态回调
    def on_configure(self, state: LifecycleState) -> TransitionCallbackReturn:
        self.get_logger().info("Configuring...")

        # 1. 创建发布者（未激活）
        self.pub = self.create_lifecycle_publisher(String, 'topic', 10)

        # 2. 创建订阅者（未激活）
        self.sub = self.create_lifecycle_subscription(
            String, 'topic', self.listener_callback, 10)

        # 3. 创建服务（未激活）
        self.srv = self.create_service(AddTwoInts, 'add_two_ints', self.service_callback)

        return TransitionCallbackReturn.SUCCESS

    def on_activate(self, state: LifecycleState) -> TransitionCallbackReturn:
        self.get_logger().info("Activating...")

        # 不需要显式激活pub/sub/srv！系统自动处理
        # 只需启动与active状态相关的逻辑（如定时器）
        self.timer = self.create_timer(1.0, self.timer_callback)
        return super().on_activate(state)

    def on_deactivate(self, state: LifecycleState) -> TransitionCallbackReturn:
        self.get_logger().info("Deactivating...")
        # 停止定时器（非生命周期实体需手动管理）
        self.timer.cancel()
        return super().on_deactivate(state)

    # 示例回调函数
    def timer_callback(self):
        msg = String()
        msg.data = "Hello from active node"
        self.pub.publish(msg)  # 仅在active状态下实际发布

    def listener_callback(self, msg):
        self.get_logger().info(f"Received: {msg.data}")  # 仅在active状态下接收

    def service_callback(self, request, response):
        response.sum = request.a + request.b  # 仅在active状态下处理请求
        return response

def main():
    rclpy.init()
    node = MyLifecycleNode()
    executor = MultiThreadedExecutor()
    
    # 3. 将节点添加到执行器
    executor.add_node(node)
    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    finally:
        executor.shutdown()
        node.destroy_node()
        rclpy.shutdown()

