#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Compile test sources with MSVC on Compiler Explorer, as the MSVC ladder would, before a push.

tools/ci-local.sh runs every Linux leg, but no MSVC, so a warning only MSVC gives, such as C4127 on
an if whose condition a stateless lambda makes constant, waited for CI to find it. This script asks
Compiler Explorer's MSVC instead:

    tools/msvc-check.py test/src/bits/bit_algorithm/bit_find_if.cpp
    tools/msvc-check.py --changed            # the tests a branch touches, and the ones mirroring its headers

Each source is compiled, not linked, with the ladder's flags (/std:c++latest /W4 /WX /permissive-) and
the test suite's definitions, on both rungs the ladder builds on a pull request, VS 2022 and the newest
MSVC. MinGW, clang-cl, Apple Clang and Emscripten stay CI's.

Compiler Explorer compiles one file, so the source goes with every header it reaches from this
repository, xstd-ints, xstd-misc, Boost and the test suite's other dependencies written into it. Which
headers those are, MSVC's preprocessor decides, so the file is cut by clang posing as it: Clang 22's
-frewrite-includes, targeting x86_64-pc-windows-msvc as MSVC 19.44 with its traditional preprocessor,
inlines what that configuration includes and leaves the macros unexpanded. The rewrite also replaces
each #if with clang's answer; those are put back, so that MSVC evaluates every condition itself. The
standard library is MSVC's own: empty stand-ins answer for its headers while clang reads, and their
#include lines are restored for MSVC. An include clang left out that MSVC takes after all is an error
naming the missing file, never a pass.

Boost.Test is replaced by a stand-in of a few dozen lines: its macros become the functions and calls
that compile every test body and instantiate every template the tests reach, without the framework
behind them, which CI compiles and which no change here touches. The include directories and
definitions come from a GCC 15 configure in build/msvc-check, which fetches the pinned dependencies as
every leg does.

Compiler Explorer is a public service, and caps a request at a few megabytes; what is sent is this
repository's own open source and its open dependencies, with blank and comment lines dropped from the
third-party headers to fit.
"""

import argparse
import json
import re
import shlex
import subprocess
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CMAKE = Path.home() / ".local" / "bin" / "cmake"
CLANG = "clang++-22"
API = "https://godbolt.org/api/compiler/{}/compile"

# The two rungs the MSVC ladder builds on a pull request: VS 2022, and VS 2026's Preview, which tracks the newest.
COMPILERS = ["vcpp_v19_44_VS17_14_x64", "vcpp_v19_latest_x64"]

# The ladder's Debug build: CMake's MSVC defaults, the test suite's warnings and /WX, and its MSVC-only definition.
FLAGS = "/std:c++latest /W4 /WX /permissive- /bigobj /EHsc /Zc:inline /Od /c /D_SCL_SECURE_NO_WARNINGS"

# Clang as MSVC 19.44 under those flags: its version, exceptions, traditional preprocessor and standard library.
CLANG_AS_MSVC = [
    "--target=x86_64-pc-windows-msvc", "-fms-compatibility-version=19.44", "-fms-extensions", "-fcxx-exceptions", "-fexceptions",
    "-std=c++23", "-nostdinc", "-nostdinc++", "-Wno-everything", "-U__clang__", "-D_MSVC_TRADITIONAL=1", "-D_MSVC_LANG=202400L",
    "-D_CPPLIB_VER=650", "-D_MSVC_STL_VERSION=143", "-D_HAS_CXX17=1", "-D_HAS_CXX20=1", "-D_HAS_CXX23=1", "-D_SCL_SECURE_NO_WARNINGS",
]

# The first path components of the headers written into the file; any other include is MSVC's own.
LIBRARIES = {"xstd", "test", "boost", "absl", "range", "meta", "concepts"}

# Where setup-toolchain.sh installs Boost, and where apt installs range-v3.
SYSTEM_ROOTS = [Path("/usr/local/include"), Path("/usr/include")]

# A request Compiler Explorer accepts, with room to spare; it refuses one of ten megabytes.
MAX_BYTES = 7_000_000

STANDARD = """
algorithm any array atomic barrier bit bitset charconv chrono codecvt compare complex concepts condition_variable coroutine
debugging deque exception execution expected filesystem flat_map flat_set format forward_list fstream functional future
generator hazard_pointer initializer_list inplace_vector iomanip ios iosfwd iostream istream iterator latch limits linalg list
locale map mdspan memory memory_resource mutex new numbers numeric optional ostream print queue random ranges ratio rcu regex
scoped_allocator semaphore set shared_mutex simd source_location span spanstream sstream stack stacktrace stdexcept stdfloat
stop_token streambuf string string_view syncstream system_error text_encoding thread tuple type_traits typeindex typeinfo
unordered_map unordered_set utility valarray variant vector version cassert cctype cerrno cfenv cfloat cinttypes climits clocale
cmath csetjmp csignal cstdarg cstddef cstdint cstdio cstdlib cstring ctime cuchar cwchar cwctype assert.h ctype.h errno.h fenv.h
float.h inttypes.h limits.h locale.h math.h setjmp.h signal.h stdarg.h stddef.h stdint.h stdio.h stdlib.h string.h time.h uchar.h
wchar.h wctype.h intrin.h immintrin.h emmintrin.h xmmintrin.h x86intrin.h
""".split()

# Boost.Test as far as the tests spell it: enough to compile each body and instantiate what it reaches.
UNIT_TEST = """\
#pragma once
#include <cstddef>
#include <tuple>
#pragma warning(push, 0)
namespace boost::unit_test {
struct test_results { std::size_t p_assertions_failed = 0; };
struct results_collector_t { auto results(int) const -> test_results { return {}; } };
inline results_collector_t results_collector;
namespace framework { inline auto current_test_case_id() -> int { return 0; } }
}
namespace msvc_check {
struct sink { template<class T> auto operator<<(T const&) -> sink& { return *this; } };
inline auto context(sink const&) -> bool { return true; }
template<class T> auto check(T const& t) -> void { static_cast<void>(static_cast<bool>(t)); }
template<class L, class R> auto eq(L const& l, R const& r) -> void { static_cast<void>(l == r); }
template<class L, class R> auto ne(L const& l, R const& r) -> void { static_cast<void>(l != r); }
template<class L, class R> auto lt(L const& l, R const& r) -> void { static_cast<void>(l < r); }
template<class L, class R> auto le(L const& l, R const& r) -> void { static_cast<void>(l <= r); }
template<class L, class R> auto ge(L const& l, R const& r) -> void { static_cast<void>(l >= r); }
template<class I, class J> auto eq_collections(I f1, I l1, J f2, J l2) -> void { for (; f1 != l1 and f2 != l2; ++f1, ++f2) { static_cast<void>(*f1 == *f2); } }
template<class List> struct each;
template<class... Ts> struct each<std::tuple<Ts...>> { template<class F> static auto run(F f) -> int { (f.template operator()<Ts>(), ...); return 0; } };
}
#pragma warning(pop)
#define BOOST_AUTO_TEST_SUITE(name, ...) namespace name {
#define BOOST_AUTO_TEST_SUITE_END() }
#define BOOST_AUTO_TEST_CASE(name, ...) void name()
#define BOOST_AUTO_TEST_CASE_TEMPLATE(name, T, list) template<class T> void name(); inline int const name##_instances = msvc_check::each<list>::run([]<class U>() -> void { name<U>(); }); template<class T> void name()
#define BOOST_CHECK(P) msvc_check::check(P)
#define BOOST_CHECK_MESSAGE(P, M) (msvc_check::check(P), static_cast<void>(msvc_check::sink{} << M))
#define BOOST_CHECK_EQUAL(L, R) msvc_check::eq(L, R)
#define BOOST_REQUIRE_EQUAL(L, R) msvc_check::eq(L, R)
#define BOOST_CHECK_NE(L, R) msvc_check::ne(L, R)
#define BOOST_CHECK_LT(L, R) msvc_check::lt(L, R)
#define BOOST_CHECK_LE(L, R) msvc_check::le(L, R)
#define BOOST_CHECK_GE(L, R) msvc_check::ge(L, R)
#define BOOST_CHECK_EQUAL_COLLECTIONS(F1, L1, F2, L2) msvc_check::eq_collections(F1, L1, F2, L2)
#define BOOST_CHECK_THROW(S, E) try { S; } catch (E const&) {}
#define BOOST_CHECK_NO_THROW(S) { S; }
#define BOOST_TEST_CONTEXT(M) if (msvc_check::context(msvc_check::sink{} << M))
#define BOOST_ERROR(M) static_cast<void>(msvc_check::sink{} << M)
"""

MARKER = re.compile(r'^# (\d+) "([^"]*)"')
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')


def configure(build):
    """The compile commands of a GCC 15 configure, which fetches the dependencies the tests include."""
    commands = build / "compile_commands.json"
    if not commands.exists():
        cmake = str(CMAKE) if CMAKE.exists() else "cmake"
        subprocess.run([cmake, "-S", str(ROOT), "-B", str(build), "-G", "Ninja", "-DCMAKE_CXX_COMPILER=g++-15", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"], check=True, stdout=subprocess.DEVNULL)
    return {Path(entry["file"]).resolve(): entry for entry in json.loads(commands.read_text())}


def roots_and_definitions(entry):
    """The include directories and definitions of one compile command, in the order the compiler searches them."""
    words = shlex.split(entry["command"]) if "command" in entry else entry["arguments"]
    roots, definitions = [], []
    for i, word in enumerate(words):
        for flag in ("-I", "-isystem"):
            if word == flag:
                roots.append(Path(words[i + 1]))
            elif word.startswith(flag) and len(word) > len(flag):
                roots.append(Path(word[len(flag):]))
        if word.startswith("-D"):
            definitions.append(word[2:])
    return roots + SYSTEM_ROOTS, definitions


def rewrite(source, roots, definitions, build):
    """The source with what clang, as MSVC, includes from the libraries written in, as clang's rewrite leaves it."""
    stub, overlay = build / "stub", build / "overlay"
    for name in STANDARD:
        (stub / name).parent.mkdir(parents=True, exist_ok=True)
        (stub / name).touch()
    (overlay / "boost" / "test").mkdir(parents=True, exist_ok=True)
    (overlay / "boost" / "test" / "unit_test.hpp").write_text(UNIT_TEST)
    (overlay / "boost" / "test" / "results_collector.hpp").write_text("#pragma once\n#include <boost/test/unit_test.hpp>\n")
    command = [CLANG, *CLANG_AS_MSVC, f"-I{overlay}", f"-I{stub}", *(f"-I{root}" for root in roots), *(f"-D{d}" for d in definitions)]
    # A header MSVC has that the list above lacks shows as a missing file; it gets a stand-in, and clang reads again.
    for _ in range(100):
        result = subprocess.run([*command, "-E", "-o", "/dev/null", str(source)], capture_output=True, text=True)
        missing = re.findall(r"fatal error: '([^']+)' file not found", result.stderr)
        if not missing:
            break
        for name in missing:
            if name.split("/")[0] in LIBRARIES:
                sys.exit(f"msvc-check: {source}: clang finds no {name}\n{result.stderr}")
            (stub / name).parent.mkdir(parents=True, exist_ok=True)
            (stub / name).touch()
    result = subprocess.run([*command, "-E", "-frewrite-includes", str(source)], capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"msvc-check: {source}: clang could not read it as MSVC\n{result.stderr}")
    return result.stdout.splitlines(), str(stub)


def restore_conditions(lines):
    """Each #if and #elif as written, where the rewrite left a disabled copy beside clang's answer."""
    out, i = [], 0
    while i < len(lines):
        if lines[i].startswith("#if 0 /* disabled by -frewrite-includes */"):
            j = i + 1
            while not lines[j].startswith("#endif /* disabled by -frewrite-includes */"):
                j += 1
            if j + 1 < len(lines) and "evaluated by -frewrite-includes" in lines[j + 1]:
                block = lines[i + 1:j]
                out.extend(block[1:-1] if lines[j + 1].startswith("#elif") else block[:-1])
                i = j + 2
                continue
        out.append(lines[i])
        i += 1
    return out


def for_msvc(lines, stub):
    """The rewrite as MSVC reads it: the standard library's includes restored, #line directives, and less third-party text."""
    out, current, number, skipped, continued, opened = [], None, 0, False, False, None
    for index, line in enumerate(lines):
        opens = line.startswith("#if defined(__CLANG_REWRITTEN_INCLUDES)")
        if line.startswith("#if 0 /* expanded by -frewrite-includes */"):
            following = INCLUDE.match(lines[index + 1]) if index + 1 < len(lines) else None
            if following and following.group(1).split("/")[0] not in LIBRARIES:
                line = "#if 1 /* a standard header in a branch clang did not take */"
        elif line.startswith("#endif") and "expanded by -frewrite-includes" in line:
            opened = None
        marker = MARKER.match(line)
        if marker:
            current, number = marker.group(2), int(marker.group(1))
            if opened is not None and current.startswith(stub):
                out[opened] = "#if 1 /* the standard header itself */"
            out.append(f'#line {number} "{current}"')
            skipped = False
            continue
        third_party = current is not None and not current.startswith((str(ROOT), stub)) and "/xstd-" not in current and not current.startswith("<")
        stripped = line.strip()
        if third_party and not continued and (not stripped or stripped.startswith("//")):
            number += 1
            skipped = True
            continue
        if skipped:
            out.append(f'#line {number} "{current}"')
            skipped = False
        continued = line.endswith("\\")
        if opens:
            opened = len(out)
        out.append(line)
        number += 1
    return out


def changed_sources(base):
    """The test sources a branch changes, and those whose path mirrors a header it changes."""
    names = subprocess.run(["git", "-C", str(ROOT), "diff", "--name-only", f"{base}...HEAD"], check=True, capture_output=True, text=True).stdout.split()
    sources = set()
    for name in names:
        if name.startswith("test/src/") and name.endswith(".cpp"):
            sources.add(ROOT / name)
        elif name.startswith("include/xstd/bits/") and name.endswith(".hpp"):
            mirror = ROOT / "test" / "src" / "bits" / name.removeprefix("include/xstd/bits/").replace("/detail/", "/")
            sources.add(mirror.with_suffix(".cpp"))
    return sorted(source for source in sources if source.exists())


def compile_on(compiler, source, flags):
    body = {"source": source, "options": {"userArguments": flags, "filters": {"execute": False}}}
    request = urllib.request.Request(API.format(compiler), data=json.dumps(body).encode(), headers={"Content-Type": "application/json", "Accept": "application/json"})
    with urllib.request.urlopen(request, timeout=600) as response:
        result = json.load(response)
    diagnostics = [line.get("text", "") for line in result.get("stderr", []) + result.get("stdout", [])]
    return result.get("code"), [text for text in diagnostics if "warning" in text or "error" in text]


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("sources", nargs="*", type=Path, help="test sources to compile")
    parser.add_argument("--changed", nargs="?", const="origin/main", metavar="BASE", help="also every test the branch's diff from BASE (origin/main) reaches")
    parser.add_argument("--compiler", action="append", help=f"a Compiler Explorer id, repeatable (default: {', '.join(COMPILERS)})")
    parser.add_argument("--build", type=Path, default=ROOT / "build" / "msvc-check", help="where to configure (default: build/msvc-check)")
    args = parser.parse_args()

    sources = [source.resolve() for source in args.sources] + (changed_sources(args.changed) if args.changed else [])
    if not sources:
        parser.error("no sources: name some, or pass --changed")
    commands = configure(args.build)

    failures = 0
    for source in dict.fromkeys(sources):
        relative = source.relative_to(ROOT)
        if source not in commands:
            print(f"FAIL {relative}: no compile command; is it a test of the suite?")
            failures += 1
            continue
        roots, definitions = roots_and_definitions(commands[source])
        lines, stub = rewrite(source, roots, definitions, args.build)
        text = "\n".join(for_msvc(restore_conditions(lines), stub))
        (args.build / relative.with_suffix(".msvc.cpp").name).write_text(text)
        if len(json.dumps(text)) > MAX_BYTES:
            print(f"FAIL {relative}: {len(text)} bytes, more than Compiler Explorer takes")
            failures += 1
            continue
        flags = " ".join([FLAGS] + [f"/D{definition}" for definition in definitions])
        for compiler in args.compiler or COMPILERS:
            code, diagnostics = compile_on(compiler, text, flags)
            print(f"{'pass' if code == 0 else 'FAIL'} {compiler:28} {relative}")
            for diagnostic in diagnostics:
                print(f"    {diagnostic}")
            failures += code != 0
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
