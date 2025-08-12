# colcon test --packages-select <name-of-pkg>
# colcon test --packages-select <name-of-pkg> --pytest-args -k name_of_the_test_function


def test_math():
    assert 2 + 2 == 4   # This should fail for most mathematical systems
