def inc(x):
    return x + 1


def test_answer():
    # assert to verify test expections
    assert inc(3) == 5


# Test function need to start with test_
def test_answer_correct():
    assert inc(3) == 4
