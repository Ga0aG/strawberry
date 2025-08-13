#!/usr/bin/env python
import rospy
import time
from std_msgs.msg import String

def callback(data):
    rospy.loginfo(rospy.get_caller_id() + "I heard %s", data.data)
    # 如果callback函数处理有延时，当前处理的错误码就不是最新的
    time.sleep(1)  # 模拟处理延时

"""
[INFO] [1754981773.639672]: /listener_886065_1754981771191I heard hello world 0
[INFO] [1754981774.642007]: /listener_886065_1754981771191[aha] I heard hello world 0
[INFO] [1754981774.642835]: /listener_886065_1754981771191I heard hello world 1
[INFO] [1754981775.643990]: /listener_886065_1754981771191[aha] I heard hello world 1
"""

def callback1(data):
    # callback的回调处理慢也会影响callback1
    rospy.loginfo(rospy.get_caller_id() + "I heard %s", data.data)

def listener():

    # In ROS, nodes are uniquely named. If two nodes with the same
    # name are launched, the previous one is kicked off. The
    # anonymous=True flag means that rospy will choose a unique
    # name for our 'listener' node so that multiple listeners can
    # run simultaneously.
    rospy.init_node('listener', anonymous=True)

    rospy.Subscriber("chatter", String, callback)
    rospy.Subscriber("chatter", String, callback1)

    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    listener()