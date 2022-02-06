#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cx_Oracle

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

# Create table
cursor.execute(
    "create table players (ID varchar2(5), Name varchar(10), Birthday timestamp, Height number(4,2))"
)

# Insert record
cursor.execute(
    "insert into players values (:ID, :Name, :Birthday, :Height)",
    ("00001", "Juliet", cx_Oracle.Timestamp(1996, 9, 17, 12, 12, 12), 1.56),
)
cursor.execute(
    "insert into players values (:ID, :Name, :Birthday, :Height)",
    ("00002", "Frank", cx_Oracle.Timestamp(1996, 9, 17, 12, 12, 12), 1.56),
)

# Update record
cursor.execute(
    "update players set Height =: h where ID = :id", {"h": 1.60, "id": "00001"}
)

# Query record
cursor.execute("select * from players")
rows = cursor.fetchall()
print(rows)
cursor.execute("select ID, Name from players")
rows = cursor.fetchall()
print(rows)

connection.commit()

pool.release(connection)
pool.close()
