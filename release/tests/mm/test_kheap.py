# import pytest

# pytestmark = pytest.mark.kheap   # falls under sys suite (memory/syscalls)


# def test_init(runner):
#     assert "PASSED*" in runner.send_serial("kheap_init")


# def test_alloc_small(runner):
#     assert "PASSED*" in runner.send_serial("kheap_alloc_small")


# def test_alloc_exact(runner):
#     assert "PASSED*" in runner.send_serial("kheap_alloc_exact")


# def test_split(runner):
#     assert "PASSED*" in runner.send_serial("kheap_split")


# def test_free_reuse(runner):
#     assert "PASSED*" in runner.send_serial("kheap_free_reuse")


# def test_coalesce(runner):
#     assert "PASSED*" in runner.send_serial("kheap_coalesce")


# def test_double_free(runner):
#     assert "PASSED*" in runner.send_serial("kheap_double_free")


# def test_invalid_free(runner):
#     assert "PASSED*" in runner.send_serial("kheap_invalid_free")


# def test_realloc_shrink(runner):
#     assert "PASSED*" in runner.send_serial("kheap_realloc_shrink")


# def test_realloc_expand(runner):
#     assert "PASSED*" in runner.send_serial("kheap_realloc_expand")


# def test_realloc_null(runner):
#     assert "PASSED*" in runner.send_serial("kheap_realloc_null")


# def test_realloc_zero(runner):
#     assert "PASSED*" in runner.send_serial("kheap_realloc_zero")


# def test_oom(runner):
#     assert "PASSED*" in runner.send_serial("kheap_oom")


# def test_stress_pattern(runner):
#     assert "PASSED*" in runner.send_serial("kheap_stress_pattern")