# dorf

C++ project built with CMake. Use the `/build` skill to configure and build.

## Coding style

Read `CODING_STYLE.md` before creating any new `.h` or `.cpp` file — it defines the copyright header, naming conventions, formatting rules, and out-parameter ordering. Apply all rules from the start; do not create a file and fix it after.

Key rules to remember:
- Every file starts with `// Copyright (c) Darrin Stewart. All rights reserved.`
- Tabs for indentation, Allman brace style
- Class members prefixed `m_`; struct members have no prefix
- Out parameters go first in function signatures
- Acronyms are treated as ordinary words (e.g., `HlslCompiler`, not `HLSLCompiler`)

### Function order in .cpp files

Definitions in a `.cpp` must appear in the same order as their declarations in the corresponding `.h`. When adding or moving functions, check the header order first and match it. Deviation is only acceptable for a specific technical reason (e.g., a helper must precede its caller with no forward declaration); if deviating, add a short comment explaining why.

## Module notes

### ArgParser (`Core/Include/Core.ArgParser/`, `Core/Source/Core.ArgParser/`)

Python-argparse-inspired CLI parser with a C-like public API. Design decisions:
- Pimpl pattern: `ArgParser` holds `ArgParserImpl*`; all state is in the `.cpp`.
- Validation uses `DORF_ASSERT` — no error codes.
- `ArgParserNumArgs` uses high sentinel values (`0x7fffffff`, etc.) so `ArgDesc::numArgs` can be typed `unsigned`, accepting either a sentinel or a literal count.
- `ArgParserArgType`, `ArgParserNumArgs`, `ArgParserAction` are all `enum class : unsigned`.
- `ArgDesc` (public) describes a single argument; `ArgParserDesc` (public) describes the parser.
- `Argument` (private, in `.cpp`) mirrors `ArgDesc` with resolved fields populated in the `ArgParserImpl` constructor loop:
  - `argType` — copied from `ArgDesc`; Auto resolved by action (StoreTrue/StoreFalse→Bool, Count→Int, else→String)
  - `numArgs` — copied from `ArgDesc`; Auto resolved by action (Store/Append→1, else→0)
  - `isPositional` — true if name does not start with `"--"`
  - `choices` — direct copy from `ArgDesc`
- `parseArgs` and `getHelpMessage` are currently stubbed with `DORF_ASSERT(false && "TODO")`.
