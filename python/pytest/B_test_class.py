class TestClass:
    # when grouping tests inside classes is that each test has a unique instance of the class.
    def test_one(self):
        assert 1 == 2

    def test_two(self):
        assert 2 == 2

    # content of test_tmp_path.py
    def test_needsfiles(tmp_path):
        print(tmp_path)
        assert 1
