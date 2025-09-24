import datetime
import logging
import sys

class MicrosecondFormatter(logging.Formatter):
    """精确到微秒的格式化器"""
    def formatTime(self, record, datefmt):
        """
        重写时间格式化方法，精确到微秒
        """
        ct = datetime.datetime.fromtimestamp(record.created)
        s = ct.strftime(datefmt)
        return s

def create_logger(name):
    logger = logging.getLogger(name)
    logger.setLevel(level=logging.DEBUG)
    formatter = logging.Formatter(
        fmt="[%(asctime)s:%(msecs)06d] [%(levelname)s] [%(filename)s:%(lineno)-4d] %(message)s",
        datefmt="%H:%M:%S",
    )
    formatter = MicrosecondFormatter(
    fmt="[%(asctime)s] [%(levelname)s] [%(filename)s:%(lineno)-4d] %(message)s",
    datefmt='%H:%M:%S.%f'  # 这个参数会被formatTime方法使用
    )
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)
    logger.propagate = False
    return logger