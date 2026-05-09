# Repository Guidelines

## Project Structure & Module Organization

This repository is for an ESPHome external component supporting AP33772S. Keep root-level files limited to documentation, repository configuration, and examples. Put implementation code under `components/ap33772s/`, reusable YAML under `packages/`, complete configs under `examples/`, and validation helpers under `tests/`.

Recommended layout:

```text
components/ap33772s/  ESPHome external component Python and C++ code
packages/             Reusable YAML substitutions, sensors, and common settings
examples/             Complete sample node configurations using external_components
tests/                Validation scripts, fixtures, and regression checks
```

Initial component files are `__init__.py`, `ap33772s.h`, and `ap33772s.cpp`. Add platform files such as `sensor.py` only for implemented ESPHome domains.

## Build, Test, and Development Commands

No project-specific scripts are committed yet. Until they exist, validate with explicit ESPHome commands:

```bash
esphome config examples/<device>.yaml
esphome compile examples/<device>.yaml
esphome run examples/<device>.yaml
```

`esphome config` validates YAML and schemas. `esphome compile` verifies firmware generation and C++ integration. `esphome run` uploads and streams AP33772S hardware logs.

## Coding Style & Naming Conventions

Use two-space YAML indentation and avoid tabs. Name YAML files with lowercase kebab-case, such as `ap33772s-devkit.yaml`. Keep substitutions near the top and prefer descriptive IDs, such as `ap33772s_voltage`.

For Python config code, follow ESPHome patterns: `CONFIG_SCHEMA`, `to_code`, explicit validation, and clear sensor options. For C++ code, use the `esphome::ap33772s` namespace, `snake_case` file names, and comments only for non-obvious register behavior.

## Testing Guidelines

Validate every example with `esphome config` before opening a pull request. For component changes, also run `esphome compile` against one representative example. When hardware is available, run `esphome run` and capture relevant logs. Add fixtures under `tests/` for schemas, generated configuration, register parsing, or hardware edge cases.

## Commit & Pull Request Guidelines

This repository has no commit history yet, so use clear, imperative messages such as `Add AP33772S sensor platform`. Keep each commit focused on one behavior, register feature, example, or documentation change.

Pull requests should include a summary, ESPHome domains changed, validation commands run, and any AP33772S hardware used for testing. Include logs only when they clarify values, I2C behavior, or failures.

## Hardware & Configuration Notes

Document I2C address, wiring assumptions, supported sensors, and register limitations in examples or README updates. Do not commit Wi-Fi credentials, API keys, OTA passwords, or device-specific secrets; use ESPHome `secrets.yaml` references.

## Agent-Specific Instructions

Do not invent generated files, register behavior, datasheet claims, build outputs, or hardware test results. When adding AP33772S features, keep examples and validation notes synchronized with the implemented component behavior.
