#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Test static variable
class TestStaticVariable:
    count = 1

    def __init__(self):
        TestStaticVariable.count += 1
        print(f"I'm {TestStaticVariable.count}")


a = TestStaticVariable()  # I'm 2
b = TestStaticVariable()  # I'm 3
