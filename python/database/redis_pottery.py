import redis
from pottery import RedisDict, RedisSimpleQueue
from IPython import embed

# Connect to Redis
r = redis.Redis(host='localhost', port=6379, db=0)
tel = RedisDict({'jack': 4098, 'sape': 4139}, redis=r, key='tel') # Create a hash key, and the keys are the field value
def getpipelien():
    return r.pipeline()
pipe = getpipelien()
pipe.set('name', 'Alice')
pipe.get('name')
responses = pipe.execute()
embed()
r.lrange
r.flushall()