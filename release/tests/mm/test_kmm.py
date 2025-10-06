# import pytest

# pytestmark = pytest.mark.sys   # falls under sys suite (memory/syscalls)

# def test_kmm_init_total(runner):
#     result = runner.send_serial("kmm_init_total")
#     assert "PASSED*" == result

# def test_kmm_reserved(runner):
#     result = runner.send_serial("kmm_reserved")
#     assert "PASSED*" == result

# def test_kmm_alloc_all(runner):
#     result = runner.send_serial("kmm_alloc_all")
#     assert "PASSED*" == result

# def test_kmm_alloc_align(runner):
#     result = runner.send_serial("kmm_alloc_align")
#     assert "PASSED*" == result

# def test_kmm_reuse(runner):
#     result = runner.send_serial("kmm_reuse")
#     assert "PASSED*" == result

# def test_kmm_double_free(runner):
#     result = runner.send_serial("kmm_double_free")
#     assert "PASSED*" == result

# def test_kmm_free_invalid(runner):
#     result = runner.send_serial("kmm_free_invalid")
#     assert "PASSED*" == result

# def test_kmm_consistency(runner):
#     result = runner.send_serial("kmm_consistency")
#     assert "PASSED*" == result

# def test_kmm_pattern(runner):
#     result = runner.send_serial("kmm_pattern")
#     assert "PASSED*" == result

# def test_kmm_oom(runner):
#     result = runner.send_serial("kmm_oom")
#     assert "PASSED*" == result


import pytest

pytestmark = pytest.mark.kmm


def assert_passed(result: str):
    """Helper: test passes if no 'FAILED:' and contains 'PASSED'."""
    assert "PASSED" in result
    assert "FAILED:" not in result


def test_kmm_init_total(runner):
    result = runner.send_serial("kmm_init_total")
    assert_passed(result)


def test_kmm_reserved(runner):
    result = runner.send_serial("kmm_reserved")
    assert_passed(result)


def test_kmm_alloc_all(runner):
    result = runner.send_serial("kmm_alloc_all")
    assert_passed(result)


def test_kmm_alloc_align(runner):
    result = runner.send_serial("kmm_alloc_align")
    assert_passed(result)


def test_kmm_reuse(runner):
    result = runner.send_serial("kmm_reuse")
    assert_passed(result)


def test_kmm_double_free(runner):
    result = runner.send_serial("kmm_double_free")
    assert_passed(result)


def test_kmm_free_invalid(runner):
    result = runner.send_serial("kmm_free_invalid")
    assert_passed(result)


def test_kmm_consistency(runner):
    result = runner.send_serial("kmm_consistency")
    assert_passed(result)


def test_kmm_pattern(runner):
    result = runner.send_serial("kmm_pattern")
    assert_passed(result)


def test_kmm_oom(runner):
    result = runner.send_serial("kmm_oom")
    assert_passed(result)
