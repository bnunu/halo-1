# OpenCE — coding standards
This document is to maintain high quality code for agents and to reduce slop.

### Comments
- **Comments explain a non-obvious *why*, never a *what*.** Well-named code already says what it does; a comment earns its place by saying something the code cannot.
- Write one for: a hidden constraint, a workaround for a specific bug, a subtle invariant, a deliberate deviation from convention, or a trap the next reader would otherwise fall into.
- Don't restate what the code does ("// loop over tokens"). Don't reference AGENTS.md rules — the rules live in AGENTS.md.
- Don't write commit-message-style narration ("added X to fix Y") — that belongs in PR descriptions and rots as the codebase evolves.
- **Length follows the reasoning, not a limit.** If the *why* genuinely takes a paragraph — a lock that must be keyed on the file rather than the instance, a guard that must not be narrowed, a measured trade with an accepted cost — write the paragraph. A reason worth recording is worth recording completely. What's forbidden is padding, not length.
- The test is whether removing the comment would let someone reasonably "fix" the code into a bug. If yes, it stays.
- **Match the repository's existing code and comment style.** Before writing, look at how the surrounding code names things and comments functions, and do the same, so new code reads like its neighbours.
- **This codebase's comment format: plain C comments only.**
  - Never `@brief`, `@param`, `@return` or other natspec/Doxygen tags, and no `/**` doc blocks or ` * ` leading-star continuation lines. Continuation lines just carry on under the text.
  - Lowercase sentence fragments that read on from the code: `/* the tunnel's version 2 (0x68 was the first) */`, a function's comment above it says what it gives back or why it exists: `/* how often the host sends again the state a client that joined since */`.
  - A reason inside a function goes in parentheses: `/* (the index buffer's binding is the vertex array's) */`.
  - Inside the decompiled `source/` tree, the comment that introduces a port addition (a new function or global, or an added block inside a decompiled function) starts with `port:` (`/* port: a client of another's game runs at the host's speed */`), so the decompiled original stays distinguishable. A reason further inside port code may stay a plain parenthesised `/* (...) */`, as upstream does.
  - Files start with the upstream header block (`/*` / FILE_NAME.C / blank line / a paragraph / `*/`), section rulers are `/* ---------- name */`.
  - Keep them short; length only where the why needs it (see above).
- **Upstream PRs:** rebase onto current testing and keep merge commits (`Merge upstream testing`) out of the branch; confirm `ninja android` and the Windows build pass after rebasing code changes, alongside the affected checks required by AGENTS.md. Documentation-only changes need document checks, not game builds.

### Git
- Don't make long commit messages
- For features, work with branches, have them reviewed both per-task and as a whole branch before merge.

### Considerations
- Ask when a gap or edge case leaves the intended behaviour, scope or a consequential tradeoff unresolved by the instructions and evidence. Explain the options and why the answer matters; continue independent work that does not depend on it.
- Communicate with concise to-do lists
- What Sapien or Guerilla (Halo's Editing Kit) could have resolved is not resolved in code (e.g. hard-coding how a shader behaves for a specific tag).
- Keep the diff lean. Suggest a lean pass before posting a PR.

### Skills to use
- Skills can help with design, planning, debugging, testing and review. Use relevant
  available skills through the tools the environment provides, reading their current
  instructions and any templates needed for the task before applying them.
- A particular skill package or tool is not a prerequisite for contributing. An
  equivalent workflow is sufficient; do not require an installation just to proceed.
- Use mutation tests selectively to check that a regression test catches the intended
  fault. Run them in isolated temporary copies, never a shared live checkout, and
  remove deliberate faults before handing the work back.

### Working method
- For reproducible bugs and new logic, prefer a failing regression test that exercises
  the real implementation, then implement the change and verify it passes. If an
  automated regression is impractical, explain why and record the alternative check.
  Documentation, investigations and upstream merges do not need artificial failing
  tests; use checks appropriate to the work without waiving AGENTS.md requirements.
- Choose tests by the failures they catch, not their size relative to the code.
  Preserve distinct boundary, lifecycle, platform, map-compatibility and network
  cases, including wire format, untrusted input and authority. Include longer
  sequences and reusable live-run tooling when needed to reproduce a regression;
  document any local-only prerequisites and keep retail assets out of the repository.
  Trim demonstrably redundant tests and controls based on coverage and measured CI
  cost, without losing meaningful checks merely to meet a line-count target.
- Investigate before you build. Read the real code AND query the actual data/state
  it runs on before designing or changing behaviour — size the work and validate the
  approach from what's actually there, not assumptions. When wiring into existing code,
  recon its data shapes and failure modes, not just function signatures.
- Work within the agreed scope. Resolve routine choices from the instructions and
  evidence, explain consequential decisions, and ask about unresolved tradeoffs or
  changes to scope before implementing them. Do not repeat an approval already given.
  Specific approval requirements in AGENTS.md still apply.
- Match effort to risk. Handle small, contained changes directly. Break substantial
  work into reviewable tasks, review each meaningful phase and the combined result
  before the final required checks. Recheck affected paths after further changes.
- Be conservative where it's costly to be wrong. For correctness/security/trust-critical
  logic, seek independent review where available and report unresolved uncertainty.
  Distinguish self-review from independent review; neither replaces validation.
- Delegation and model selection depend on the task, available tools and the user's
  instructions. No particular provider, model or subagent workflow is required.
  Delegate only when permitted and when the separate context, tests or review justify
  the handoff cost. Working directly still requires the same review and verification.
- Verify for real; report honestly. Green tests aren't proof it works — verify with a
  real run / real output where feasible. State outcomes faithfully: failures, caveats,
  and skipped steps included.
