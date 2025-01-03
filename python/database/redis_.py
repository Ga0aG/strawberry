import json
import redis
from datetime import datetime, timedelta
from dataclasses import dataclass, asdict
from IPython import embed
from serde import Validate, Serialize

@dataclass
class User:
    id: str
    name: str
    num: int
    createdDatetime: int

# Connect to Redis
redisClient = redis.Redis(host='localhost', port=6379, db=0)

# Create two user objects
user1 = User(id='user1', name='Alice', num=3, createdDatetime=eval(datetime.now().strftime("%Y%m%d%H%M%S%f")))
user2 = User(id='user2', name='Bob', num=1, createdDatetime=eval((datetime.now()+timedelta(microseconds=300)).strftime("%Y%m%d%H%M%S%f")))
user3 = User(id='user3', name='Julie', num=4, createdDatetime=eval((datetime.now()+timedelta(microseconds=100)).strftime("%Y%m%d%H%M%S%f")))

# Create a set to store user1's friends
redisClient.sadd('user:developers', 'user:user1')
redisClient.sadd('user:developers', 'user:user2')
redisClient.sadd('user:developers', 'user:user3')

# Store the user objects in Redis
redisClient.hmset('user:user1', asdict(user1))
redisClient.hmset('user:user2', asdict(user2))
redisClient.hmset('user:user3', asdict(user3))

# Retrieve user1's friends
developers = redisClient.smembers('user:developers')

# Get the friend objects
for developerId in developers:
    developer = redisClient.hgetall(f'{developerId.decode()}')
    print(developer)

redisClient.set('test:user1', json.dumps(Serialize(user1)))
restoreUser = Validate(json.loads(redisClient.get('test:user1')), User, allowTransform=True)
embed()