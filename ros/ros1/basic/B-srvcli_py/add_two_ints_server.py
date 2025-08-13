#!/usr/bin/env python

import time

from std_srvs.srv import Empty
import rospy

def handle_add_two_ints(req):
    time.sleep(10)
    print(f"Finished handling request")

def add_two_ints_server():
    rospy.init_node('add_two_ints_server')
    s = rospy.Service('add_two_ints', Empty, handle_add_two_ints)
    print("Ready to add two ints.")
    rospy.spin()

if __name__ == "__main__":
    add_two_ints_server()
