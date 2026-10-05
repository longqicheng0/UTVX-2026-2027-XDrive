# Learning tooling setup

Installed 2026-09-28, workspace-only. This setup creates no application code, test harness, PROS project, or Git repository and makes no reference-repository edits.

| Capability | Source version | Installation location |
| --- | --- | --- |
| Matt Pocock grill-me | commit c55ee46073ed923f86ce59a5eb3b6d895095d1b7 | `.agents/skills/grill-me/SKILL.md` |
| Matt Pocock grilling | same commit | `.agents/skills/grilling/SKILL.md` |
| HVE-derived X-drive tutor | HVE package 3.2.2, commit a146d8d01ad1e38f82d5d8eebb7fa8d1d88c4092 | `.agents/skills/xdrive-hve-tutor/SKILL.md` |
| Workspace teaching contract | local adaptation | `AGENTS.md` |

HVE's pin is an inspected development-tip commit, not a stable release claim. Matt's version is identified by commit. Fingerprints and licenses are recorded in [sources.json](sources.json) and `upstream/`.

## Installer and dependencies

Matt's files were installed unmodified using the host's supported installer:

```sh
python3 /Users/elong/.codex/skills/.system/skill-installer/scripts/install-skill-from-github.py --repo mattpocock/skills --ref c55ee46073ed923f86ce59a5eb3b6d895095d1b7 --path skills/productivity/grill-me skills/productivity/grilling --dest /Users/elong/Desktop/UTVX-2026-2027-X-Drive/.agents/skills
```

grill-me depends on grilling, which is installed. Both are instruction-only, with no runtime packages, scripts, or MCP dependencies. The upstream repository's general setup configures issue tracking/triage/domain documents for its engineering suite; these productivity skills do not call it. The literal Skill-tool handoff is bridged through AGENTS.md to reading the installed dependency. Batch questions and immediate recommended answers are overridden by the user's teaching rules.

Official HVE installation documents the VS Code Copilot extension, Copilot CLI plugin, and selective adoption. No documented native Codex installation was found. A Copilot extension does not activate HVE in Codex. This workspace installs a local adaptation of the five lifecycle responsibilities, using learner implementation and conversational checkpoints.

Full upstream installer scripts assume VS Code/Insiders and use Bash/jq or PowerShell. Full rpi-plan also requires a PowerShell assessment helper and rpi-plan-critique. Those components are not installed or advertised as operational here. The selected text adaptation needs no npm packages, PowerShell, MCP server, extension, telemetry hook, or background service. npm ci is for developing the full HVE source project.

## Discovery and activation

Host binary: `codex-cli 0.155.0-alpha.16.3`. Codex documents `.agents/skills` as a local discovery path and watches skill changes. Skills should be available on the next turn; restart Codex/reopen this workspace if the UI does not refresh. A new session may be needed for automatic inclusion of AGENTS.md; the setup session reads it directly.

Actual loader evidence is in [discovery.json](discovery.json): the installed binary's app-server skills/list, scoped to this workspace with forceReload. This verifies parsing/discovery, not future tutoring behaviour or the current UI's refresh. The check does not launch a model turn. No native Copilot slash command or unavailable Skill tool is claimed.

Result: all three skills were returned as enabled with repo scope and zero loader errors. The created tutor skill and upstream grilling passed the host's skill validator. The loader accepted grill-me's upstream frontmatter as installed. Both requested SKILL.md files were compared byte-for-byte against their pinned official sources. Recorded source hashes and authored local links passed verification. An independent read-only review found no material conflicts with the user's teaching constraints.

## Inspection

- Workspace initially empty; reference folder accessible and treated read-only.
- Apple Clang 21.0.0, arm64 macOS; GNU Make 3.81; Git 2.54.0.
- Python 3.10.20 with PyYAML 6.0.3; Node 25.6.1; npm 11.9.0.
- pros, cmake, and code not found on PATH; they may exist elsewhere.
- No desktop compilation, PROS build/upload, or hardware test has run. Setup choice waits for the hardware-availability answer.
- Reference main/project.pros records V5, kernel 3.8.3, okapilib 4.8.0, irk-radio 1.0.2. These are reference metadata, not selected project dependencies.
- Reference main/src/config.cpp declares eight drive motors, two tracking rotation devices, and an IMU. Ports, reversal flags, geometry, and actual wiring remain independently unverified.

## Sources reviewed

- [Matt skills README](https://github.com/mattpocock/skills/blob/c55ee46073ed923f86ce59a5eb3b6d895095d1b7/README.md)
- [grill-me](https://github.com/mattpocock/skills/blob/c55ee46073ed923f86ce59a5eb3b6d895095d1b7/skills/productivity/grill-me/SKILL.md) and [grilling](https://github.com/mattpocock/skills/blob/c55ee46073ed923f86ce59a5eb3b6d895095d1b7/skills/productivity/grilling/SKILL.md)
- [HVE installation](https://github.com/microsoft/hve-core/blob/a146d8d01ad1e38f82d5d8eebb7fa8d1d88c4092/docs/getting-started/install.md)
- [HVE customization](https://github.com/microsoft/hve-core/blob/a146d8d01ad1e38f82d5d8eebb7fa8d1d88c4092/docs/customization/forking.md)
- [HVE RPI lifecycle](https://github.com/microsoft/hve-core/blob/a146d8d01ad1e38f82d5d8eebb7fa8d1d88c4092/docs/rpi/README.md)
- [Codex skill discovery](https://learn.chatgpt.com/docs/build-skills) and [app-server protocol](https://learn.chatgpt.com/docs/app-server)

The first milestone is physical X-drive layout and coordinate conventions. No convention is accepted or implemented. The single pending question is whether robot/controller hardware is available.
