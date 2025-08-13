#!/usr/bin/env python

import sys
import rospy
from std_srvs.srv import Empty
from threading import Thread

def add_two_ints_client(x, y):
    rospy.wait_for_service('add_two_ints')
    try:
        add_two_ints = rospy.ServiceProxy('add_two_ints', Empty)
        resp1 = add_two_ints()
        return x+y
    except rospy.ServiceException as e:
        print("Service call failed: %s"%e)

def usage():
    return "%s [x y]"%sys.argv[0]

def tick_tock():
    count = 0
    while not rospy.is_shutdown():
        rospy.sleep(1)
        print(f"Tick...{count}")
        count += 1

if __name__ == "__main__":
    thread = Thread(target=tick_tock, daemon=True)
    thread.start()
    if len(sys.argv) == 3:
        x = int(sys.argv[1])
        y = int(sys.argv[2])
    else:
        print(usage())
        sys.exit(1)
    print("Requesting %s+%s"%(x, y))
    # ros1的service call相当于一个I/O操作，所以不会阻塞到tick_tock线程
    print("%s + %s = %s"%(x, y, add_two_ints_client(x, y)))