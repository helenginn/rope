# Tests

Testing is implemented via [doctest](https://github.com/doctest/doctest).

## Setup

- Installed through `conanfile.py`.
- Toggled via meson build flag `enable_tests=true` / `enable_tests=false`.
- Setup scripts ask about this (included by default).

## Structure

- `tests/` at repo root, entry point for doctest, used for end-to-end tests.
- Inline unit tests are also supported directly in `.cpp` files anywhere in the repo.

```cpp
#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>
// test logic block
#endif
```

## Examples

Two tests exist so far as a trial:

- `vagabond/core/forces/Rod.cpp` (inline unit tests)
- `tests/smoke_tests.cpp` (e2e test)

## Writing tests

doctest offers three assertion levels, used inside `TEST_CASE`:
- `CHECK(expr)` -- logs a failure but keeps test case running.
- `REQUIRE(expr)` -- aborts current test case on failure.
- `WARN(expr)` -- only prints a message on failure, does not mark the test as failed.

```cpp
#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>
#include <vector>

TEST_CASE("rod applies correct force") {
  std::vector<int> v{5, 3, 1, 4, 2};

  REQUIRE(!v.empty()); // precondition
  CHECK(v.front() == 1);
  CHECK(v.back() == 5);
  WARN(v.capacity() <= 16); // not wrong
}
```

## Running 

```bash
meson test -C <your-build-dir>
```

For failing tests, get the stdout/stderr instead of just pass/fail:

```bash
meson test -C <your-build-dir> --print-errorlogs
```

(--print-errorlogs is a meson flag, not a doctest one)

You can also pass doctest's own flag (--success)

```bash
meson test -C <your-build-dir> --test-args="--success"
```

## Tooling

Make sure you use the right tooling. Meson can complain about a version number mismatch. The setup scripts use the conan-provided tools. You can source them via:

```bash
source <your-build-dir>/conanbuild.sh
```
Alternatively, you can reconfigure the build dir via:
```bash
meson setup --reconfigure <your-build-dir>
```
