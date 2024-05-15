# -*- coding: UTF-8 -*-
from datetime import datetime, timedelta
from sqlalchemy import create_engine
from sqlalchemy import Column, Integer, String, DateTime, ForeignKey
from sqlalchemy.orm import mapped_column, Mapped, declarative_base, sessionmaker, relationship
from IPython import embed

# 连接引擎
engine = create_engine('sqlite:///:memory:', echo=True)

# 声明映射，在Python中创建的一个类，对应着数据库中的一张表，类的每个属性，就是这个表的字段名。
Base = declarative_base()

# ======= RelationShip =========
# **** Case A. One to Many *****

# class Parent(Base):
#     __tablename__ = "parent_table"
#     id = Column(Integer, primary_key=True)
#     children = relationship("Child")


# class Child(Base):
#     __tablename__ = "child_table"
#     id = Column(Integer, primary_key=True)
#     parent_id = Column(Integer, ForeignKey("parent_table.id"))

# **** Case B. One to One *****
# class Parent(Base):
#     __tablename__ = "parent_table"
#     id = Column(Integer, primary_key=True)

#     # one-to-many collection
#     children = relationship("Child", back_populates="parent")


# class Child(Base):
#     __tablename__ = "child_table"
#     id = Column(Integer, primary_key=True)
#     parent_id = Column(Integer, ForeignKey("parent_table.id"))

#     # many-to-one scalar
#     parent = relationship("Parent", back_populates="children")

# **** Case C. Many to one *****
class Parent(Base):
    __tablename__ = "parent_table"
    id = Column(Integer, primary_key=True)
    child_id = Column(Integer, ForeignKey("child_table.id"))
    child = relationship("Child")


class Child(Base):
    __tablename__ = "child_table"
    id = Column(Integer, primary_key=True)


Parent.metadata.create_all(engine)
Child.metadata.create_all(engine)
Session = sessionmaker(bind=engine)
# 实例化
session = Session()
# mom = Parent(id=1)
mom = Parent(id=1, child_id=2)
session.add(mom)
dad = Parent(id=3, child_id=2)
session.add(dad)
# child1 = Child(id=2,parent_id=1)
child1 = Child(id=2)
# child2 = Child(id=3,parent_id=1)
session.add(child1)
# session.add(child2)
session.commit()
# # SELECT
parents = session.query(Parent).all()
child = parents[0].child

embed()
session.close()
engine.dispose()
