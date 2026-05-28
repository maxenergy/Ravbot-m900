# Repository Guidelines

## Project Structure & Module Organization

RavBot is a C++17 application with TypeScript companion packages. Core C++
sources live in `src/`, with public headers in `include/ravbot/`. Tests are
in `tests/` and generally mirror feature areas, for example
`test_provider_registry.cpp`. The dashboard UI is in `ui/`, the Node.js plugin
sidecar is in `sidecar/`, documentation is in `website/`, scripts are in
`scripts/`, and bundled skills/assets are in `assets/`.

## Build, Test, and Development Commands

- `./scripts/build.sh --tests`: configure and build C++ with tests enabled.
- `cmake --preset strict-clang && cmake --build --preset strict-clang --parallel`:
  run the strict Clang/Ninja build used by hardening CI.
- `ctest --preset strict-clang`: run C++ tests for the strict preset.
- `./scripts/format-code.sh --check`: verify C++ formatting; omit `--check` to
  rewrite files with `clang-format-18`.
- `cd ui && npm ci && npm test`: install and run dashboard Vitest tests.
- `cd sidecar && npm ci && npm run build && npm test`: build and test sidecar.
- `cd website && npm ci && npm run docs:dev`: run local VitePress docs.

## Coding Style & Naming Conventions

C++ follows `.clang-format`: Google style, 2-space indentation, no tabs,
80-column limit, attached braces, sorted includes, and C++17. Keep headers under
`include/ravbot/<area>/` aligned with files under `src/<area>/`. Use
`snake_case` for C++ files and tests. TypeScript uses ESM, 2-space indentation,
and test files named `*.test.ts` or `*.node.test.ts`.

## Testing Guidelines

Add focused regression tests for behavior changes. C++ tests should be named
`tests/test_<feature>.cpp` and be wired through the existing CMake test target.
UI and sidecar tests use Vitest; place node-only UI tests under `ui/src/**` with
the `.node.test.ts` suffix. Run the smallest relevant suite first, then run CTest
or package-level `npm test` before opening a PR.

## Commit & Pull Request Guidelines

Recent history uses Conventional Commit-style subjects such as `fix(ui): ...`,
`test: ...`, `style: ...`, and `chore(deps): ...`; keep subjects imperative and
scoped when useful. Pull requests should target `main` or the appropriate feature
branch, describe the change, link issues, identify the change type, list local
tests run, and include screenshots for dashboard UI changes. Update docs when
commands, configuration, or user-visible behavior changes.

## Security & Configuration Tips

Do not commit real provider tokens, gateway auth tokens, or local
`~/.ravbot` data. Use `config.example.json` and `scripts/env.example.txt` as
templates, and keep secrets in local environment variables or user config files.
