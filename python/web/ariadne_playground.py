from ariadne import ObjectType, gql, make_executable_schema
from ariadne.asgi import GraphQL
from typing import Optional

# Define GraphQL schema
type_defs = gql(
    """
    type Query {
        hello: String!
        user(username: String! age: Int): User
        users: [User!]!
    }
    type Mutation {
        addUser(firstName: String!, secondName: String!, age: Int!): User!
        updateUser(firstName: String!, secondName: String!, age: Int): User
        deleteUser(firstName: String!, secondName: String!, age: Int): User
    }
    type User {
        username: String!
        age: Int!
    }
    """
)

# Sample user data (mock database)
USER_DATA = [
    {"firstName": "Julin", "secondName": "Lara", "age": 25},
    {"firstName": "Alice", "secondName": "Smith", "age": 30},
    {"firstName": "Bob", "secondName": "Brown", "age": 35},
    {"firstName": "Nan", "secondName": "Hang", "age": 35},
]

# Define resolvers for Query type
query_type = ObjectType("Query")


@query_type.field("hello")
def resolve_hello(_, info):
    request = info.context["request"]
    user_agent = request.headers.get("user-agent", "guest")
    return f"Hello, {user_agent}!"


@query_type.field("user")
def resolve_user(_, __, username, age: Optional[int] = None):
    # Find user by username
    for user in USER_DATA:
        full_name = f"{user['firstName']} {user['secondName']}"
        if full_name == username and (not age or age == user['age']):
            return user
    return None  # Return None if user is not found


@query_type.field("users")
def resolve_users(*_):
    # Return the list of all users
    return USER_DATA


# Define resolvers for User type
user_type = ObjectType("User")


@user_type.field("username")
def resolve_username(obj, *_):
    return f"{obj['firstName']} {obj['secondName']}"


@user_type.field("age")
def resolve_age(obj, *_):
    return obj["age"]

# Define resolvers for Mutation type
mutation_type = ObjectType("Mutation")
@mutation_type.field("addUser")
def resolve_add_user(_, __, firstName: str, secondName: str, age: int):
    # Add a new user to the USER_DATA list
    new_user = {"firstName": firstName, "secondName": secondName, "age": age}
    USER_DATA.append(new_user)
    return new_user

@mutation_type.field("updateUser")
def resolve_update_user(_, __, firstName: str, secondName: str, age: int):
    # Add a new user to the USER_DATA list
    for user in USER_DATA:
        if user['firstName'] == firstName and user['secondName'] == secondName:
            user['age'] = age
            return user
    return None

@mutation_type.field("deleteUser")
def resolve_update_user(_, __, firstName: str, secondName: str, age: int):
    # Add a new user to the USER_DATA list
    for ind, user in enumerate(USER_DATA):
        if user['firstName'] == firstName and user['secondName'] == secondName and user['age'] == age:
            USER_DATA.pop(ind)
            return user
    return None

# Create executable schema
schema = make_executable_schema(type_defs, [query_type, user_type, mutation_type])
app = GraphQL(schema, debug=True)

"""
# QUERY

## Get User by username and age
query getuser{
  user(username: "Alice Smith" age: 30) {
    username
    age
  }
}

## Get all users
query getusers{
  users {
    username
    age
  }
}

# MUTATION

## Add user
mutation adduser{
  addUser(firstName: "Alice", secondName: "Smith", age: 30) {
    username
    age
  }
}

## Update user
mutation updateuser{
  updateUser(firstName: "Alice", secondName: "Smith", age: 15) {
    username
    age
  }
}

# Search and filter
"""

