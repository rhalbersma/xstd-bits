#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""A block's shifts by a std::size_t count, over the word operations of xstd-ints' operations.py.

xstd-ints' kernels shift a std::size_t by a std::size_t, so its operations take every count as a word. The kernels
here shift a block of 8 to 128 bits by a std::size_t count, so a count gets a width of its own, 64 bits, and a shift
is taken as C++ takes it: the block is promoted to int when narrower, so a shift is defined for a count below the
promoted width, and the bits it moves past the block's own width are cut when the result converts back.

operations.py comes from the pinned xstd-ints, imported from where FetchContent put it: see load().
"""

import importlib.util
import os
import sys

COUNT_BITS = 64
INT_BITS = 32


def load(directory=None):
    """xstd-ints' operations.py, from the directory given or from XSTD_INTS_PROOFS, under a name of its own.

    Loaded by path, and not by putting the directory on sys.path, where its kernels.py would shadow this one's.
    """
    directory = directory or os.environ.get("XSTD_INTS_PROOFS")
    if not directory:
        raise SystemExit("set XSTD_INTS_PROOFS to xstd-ints' test/proofs directory, or pass --ints-proofs")
    spec = importlib.util.spec_from_file_location("xstd_ints_operations", os.path.join(directory, "operations.py"))
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def promoted_width(width):
    """The width a shift operates at: int's for a block narrower than int, the block's own otherwise."""
    return max(width, INT_BITS)


def interpretations(operations):
    """The Z3 and integer interpretations, each extended by counts and shifts by a count."""
    import z3

    class Z3Blocks(operations.Z3Ops):
        def count(self, value):
            return value if isinstance(value, z3.BitVecRef) else z3.BitVecVal(value, COUNT_BITS)

        def count_ult(self, a, b):
            return z3.ULT(self.count(a), self.count(b))

        def count_ule(self, a, b):
            return z3.ULE(self.count(a), self.count(b))

        def count_eq(self, a, b):
            return self.count(a) == self.count(b)

        def count_add(self, a, b):
            return self.count(a) + self.count(b)

        def count_sub(self, a, b):
            return self.count(a) - self.count(b)

        def count_mul(self, a, b):
            return self.count(a) * self.count(b)

        def count_div(self, a, b):
            return z3.UDiv(self.count(a), self.count(b))

        def count_mod(self, a, b):
            return z3.URem(self.count(a), self.count(b))

        def not_(self, condition):
            return z3.Not(condition)

        def or_(self, *conditions):
            return z3.Or(*conditions)

        def bool_ite(self, condition, then, otherwise):
            return z3.If(condition, then, otherwise)

        def low_word(self, n):
            n = self.count(n)
            return z3.Extract(self.width - 1, 0, n) if self.width <= COUNT_BITS else z3.ZeroExt(self.width - COUNT_BITS, n)

        def shl_by(self, block, n):
            return z3.If(self.count_ult(n, self.width), self.word(block) << self.low_word(n), self.word(0))

        def shr_by(self, block, n):
            return z3.If(self.count_ult(n, self.width), z3.LShR(self.word(block), self.low_word(n)), self.word(0))

    class IntBlocks(operations.IntOps):
        def count(self, value):
            return value

        def count_ult(self, a, b):
            return a < b

        def count_ule(self, a, b):
            return a <= b

        def count_eq(self, a, b):
            return a == b

        def count_add(self, a, b):
            return a + b

        def count_sub(self, a, b):
            return (a - b) % (1 << COUNT_BITS)

        def count_mul(self, a, b):
            return a * b

        def count_div(self, a, b):
            return a // b

        def count_mod(self, a, b):
            return a % b

        def not_(self, condition):
            return not condition

        def or_(self, *conditions):
            return any(conditions)

        def bool_ite(self, condition, then, otherwise):
            return then if condition else otherwise

        def shl_by(self, block, n):
            return self.shl(self.word(block), n) if n < self.width else self.word(0)

        def shr_by(self, block, n):
            return self.lshr(self.word(block), n) if n < self.width else self.word(0)

    return Z3Blocks, IntBlocks
