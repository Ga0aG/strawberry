import pytest

""" 
In testing, a fixture provides a defined, reliable and consistent context for the tests. 
This could include environment (for example a database configured with known parameters) 
or content (such as a dataset). 
The services, state, or other operating environments set up by fixtures are accessed by 
test functions through arguments.
"""


class Fruit:
    def __init__(self, name):
        self.name = name

    def __eq__(self, other):
        return self.name == other.name

"""
Scope: sharing fixtures across classes, modules, packages or session

function: the default scope, the fixture is destroyed at the end of the test.
class: the fixture is destroyed during teardown of the last test in the class.
module: the fixture is destroyed during teardown of the last test in the module.
package: the fixture is destroyed during teardown of the last test in the package.
session: the fixture is destroyed at the end of the test session.
"""

@pytest.fixture
def my_fruit():
    return Fruit("apple")


@pytest.fixture
def fruit_basket(my_fruit):
    return [Fruit("banana"), my_fruit]


def test_my_fruit_in_basket(my_fruit, fruit_basket):
    assert my_fruit in fruit_basket
