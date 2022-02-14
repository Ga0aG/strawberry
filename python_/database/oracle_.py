#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cx_Oracle

# I. CRUD, create, read, update, delete
cx_Oracle.init_oracle_client(lib_dir="/usr/lib/oracle/21/client64/lib")
pool = cx_Oracle.SessionPool(
    user="system",
    password="oracle",
    dsn="localhost:49161/xe",
    min=2,
    max=5,
    increment=1,
    encoding="UTF-8",
)

connection = pool.acquire()
cursor = connection.cursor()

# Drop table if exist
try:
    cursor.execute("drop table players")
except cx_Oracle.DatabaseError:
    pass
try:
    cursor.execute("drop table customers")
except cx_Oracle.DatabaseError:
    pass
try:
    cursor.execute("drop table orders")
except cx_Oracle.DatabaseError:
    pass

# Create table
cursor.execute(
    "create table players (ID varchar2(5), Name varchar(10), Birthday timestamp, Height number(4,2))"
)

# Insert record
cursor.execute(
    "insert into players values (:ID, :Name, :Birthday, :Height)",
    ("00001", "Juliet1", cx_Oracle.Timestamp(1996, 9, 17, 12, 12, 12), 1.56),
)
cursor.execute(
    "insert into players values (:ID, :Name, :Birthday, :Height)",
    ("00002", "Frank1", cx_Oracle.Timestamp(1996, 9, 17, 12, 12, 12), 1.56),
)

# Update record
cursor.execute(
    "update players set Height =: h where ID = :id and Name = :name",
    {"h": 1.60, "id": "00001", "name": "Juliet1"},
)

# Query record
cursor.execute("select * from players")
rows = cursor.fetchall()
print(rows)
cursor.execute("select ID, Name from players")
rows = cursor.fetchall()
print(rows)

# Delete record
cursor.execute("delete from players where ID = :id", {"id": "00002"})

# II. Example - Customer Order Management
cursor.execute(
    "create table customers (ID varchar2(5), Name varchar(10), constraint unique_id1 primary key (ID))"
)
cursor.execute(
    "insert into customers values (:ID, :Name)",
    ("10001", "Juliet2"),
)
cursor.execute(
    "insert into customers values (:ID, :Name)",
    ("10002", "Frank2"),
)
try:
    cursor.execute(
        "insert into customers values (:ID, :Name)",
        ("10002", "Frank2"),
    )
except:
    print("Duplicate ID is not allowed")

cursor.execute(
    "create table orders (ID varchar2(5), Customer_id varchar(10), Ord_date timestamp, constraint unique_id2 primary key (ID))"
)
cursor.execute(
    "insert into orders values (:ID, :customer_id, :Ord_date)",
    ("00001", "10001", cx_Oracle.Timestamp(2022, 2, 11, 12, 12, 12)),
)
cursor.execute(
    "insert into orders values (:ID, :customer_id, :Ord_date)",
    ("00002", "10002", cx_Oracle.Timestamp(2022, 2, 12, 12, 12, 12)),
)

cursor.execute(
    "select orders.ID, customers.Name from orders, customers where orders.customer_id = customers.ID"
)
rows = cursor.fetchall()
print(rows)

connection.commit()

pool.release(connection)
pool.close()
