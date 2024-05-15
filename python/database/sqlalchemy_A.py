# -*- coding: UTF-8 -*-
from datetime import datetime, timedelta
from sqlalchemy import create_engine
from sqlalchemy import Column, Integer, String, DateTime
from sqlalchemy.orm import DeclarativeBase, Mapped, declarative_base, sessionmaker
from IPython import embed
# https://zhuanlan.zhihu.com/p/387078089

# 连接引擎
engine = create_engine('sqlite:///:memory:', echo=True)

# 声明映射，在Python中创建的一个类，对应着数据库中的一张表，类的每个属性，就是这个表的字段名。
Base = declarative_base()
# class Base(DeclarativeBase):
#     pass
class User(Base):
    __tablename__ = 'users'
    id = Column(Integer, primary_key=True)
    # name = Column(String)
    name: Mapped[str]
    fullname = Column(String)
    nickname = Column(String)
    created_datetime: Mapped[datetime]
    def __repr__(self):
       return "<User(name='%s', fullname='%s', nickname='%s', createdTime='%s')>" % (
                            self.name, self.fullname, self.nickname, self.created_datetime)

User.metadata.create_all(engine)
"""
In [27]: User.metadata.sorted_tables
Out[27]: [Table('users', MetaData(), Column('id', Integer(), table=<users>, primary_key=True, nullable=False), Column('name', String(), table=<users>), Column('fullname', String(), table=<users>), Column('nickname', String(), table=<users>), schema=None)]
"""

Session = sessionmaker(bind=engine)
now = datetime.now()
# 实例化
session = Session()
ed_user = User(name='ed', fullname='Ed Jones', nickname='edsnickname', created_datetime = now+timedelta(microseconds=3))
session.add(ed_user)
ed_user = User(name='ad', fullname='ad Jones', nickname='adsnickname', created_datetime = now+timedelta(microseconds=1))
session.add(ed_user)
session.commit()
# SELECT
our_user = session.query(User).filter_by(name='ed').first()
# 回滚
# session.rollback()
# 查询
for instance in session.query(User).order_by(User.id):
    print(instance.name, instance.fullname)
query = session.query(User).order_by(User.created_datetime)
# query.filter() 过滤
# query.filter_by() 根据关键字过滤
# query.all() 返回列表
# query.first() 返回第一个元素
# query.one() 有且只有一个元素时才正确返回
# query.one_or_none()，类似one，但如果没有找到结果，则不会引发错误
# query.scalar()，调用one方法，并在成功时返回行的第一列
# query.count() 计数
# query.order_by() 排序

embed()
session.close()
engine.dispose()
exit()
# 常用的筛选器
# 等于
query.filter(User.name == 'ed')

# 不等于
query.filter(User.name != 'ed')

# like和ilike
query.filter(User.name.like('%ed%'))
query.filter(User.name.ilike('%ed%')) # 不区分大小写

# in
query.filter(User.name.in_(['ed', 'wendy', 'jack']))
query.filter(User.name.in_(
    session.query(User.name).filter(User.name.like('%ed%'))
))
# not in
query.filter(~User.name.in_(['ed', 'wendy', 'jack'])) 

# is
query.filter(User.name == None)
query.filter(User.name.is_(None))

# is not
query.filter(User.name != None)
query.filter(User.name.is_not(None))

# and
from sqlalchemy import and_
query.filter(and_(User.name == 'ed', User.fullname == 'Ed Jones'))
query.filter(User.name == 'ed', User.fullname == 'Ed Jones')
query.filter(User.name == 'ed').filter(User.fullname == 'Ed Jones')

# or
from sqlalchemy import or_
query.filter(or_(User.name == 'ed', User.name == 'wendy'))

# match
query.filter(User.name.match('wendy'))

session.close()
engine.dispose()