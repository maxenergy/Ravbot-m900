# Frontend I18n Plan

This plan tracks remaining frontend internationalization work for `ui/src`.
Use it as the `/goal` backlog. For each batch, move visible strings into
`ui/src/i18n/locales/{en,zh-CN,zh-TW,pt-BR}.ts`, use `t(...)` at call sites,
and keep locale keys aligned with `ui/src/i18n/test/translate.test.ts`.

## Verification

Run after each batch:

```bash
cd ui
npx vitest run src/i18n/test/translate.test.ts --browser.enabled=false --environment node
npm run build
```

Browser tests require Playwright browsers to be installed locally.

## Already Partially Covered

- `ui/src/i18n/lib/translate.ts`
- `ui/src/i18n/test/translate.test.ts`
- `ui/src/ui/app-render.ts`
- `ui/src/ui/views/overview.ts`
- `ui/src/ui/views/chat.ts`
- `ui/src/ui/views/sessions.ts`
- `ui/src/ui/views/instances.ts`
- `ui/src/ui/views/logs.ts`
- `ui/src/ui/views/channels.ts`
- `ui/src/ui/views/channels.shared.ts`
- `ui/src/ui/views/channels.discord.ts`
- `ui/src/ui/views/channels.googlechat.ts`
- `ui/src/ui/views/channels.imessage.ts`
- `ui/src/ui/views/channels.signal.ts`
- `ui/src/ui/views/channels.slack.ts`
- `ui/src/ui/views/channels.telegram.ts`
- `ui/src/ui/views/channels.whatsapp.ts`

These files may still contain brand names, protocol names, key names, examples,
or technical values that should remain untranslated.

## Batch 1: Finish Channels And Nostr

Primary files:

- `ui/src/ui/views/channels.nostr.ts`
- `ui/src/ui/views/channels.nostr-profile-form.ts`
- `ui/src/ui/views/channels.config.ts`

Scope:

- Nostr status labels: `Running`, `Configured`, `Public Key`, `Last inbound`,
  `Last start`.
- Nostr profile labels: `Profile`, `Name`, `Display Name`, `About`, empty
  profile help text, image alt text.
- Nostr profile form: `Edit Profile`, `Username`, `Display Name`, `Avatar URL`,
  `Advanced`, `Banner URL`, `Website`, `NIP-05 Identifier`,
  `Lightning Address`, save/import/advanced buttons, placeholders, help text.
- Channel config warnings and actions: schema unavailable messages, `Save`,
  `Saving...`.

Notes:

- Keep protocol identifiers like `NIP-05`, `NIP-04`, `LUD-16`, relay names,
  public keys, and URLs untranslated.
- Reuse existing `common.*` and `channels.status.*` keys where possible.

## Batch 2: Theme, Markdown Sidebar, Chat Tool Cards

Primary files:

- `ui/src/ui/app-render.helpers.ts`
- `ui/src/ui/views/markdown-sidebar.ts`
- `ui/src/ui/chat/tool-cards.ts`
- `ui/src/ui/chat/copy-as-markdown.ts`
- `ui/src/ui/chat/grouped-render.ts`

Scope:

- Theme toggle labels and titles: `Theme`, `System`, `Light`, `Dark`.
- Markdown sidebar: `Tool Output`, `Close sidebar`, `No content available`,
  raw-text actions if present.
- Tool card visible status strings, especially `Completed`.
- Copy button states: `Copy as markdown`, `Copied`, `Copy failed`.
- Message/group labels if shown to users.

Notes:

- Preserve keyboard names such as `Enter`.
- Preserve roles if they are data values, but translate labels shown in UI.

## Batch 3: Agents

Primary files:

- `ui/src/ui/views/agents.ts`
- `ui/src/ui/views/agents-panels-status-files.ts`
- `ui/src/ui/views/agents-panels-tools-skills.ts`
- `ui/src/ui/views/agents-utils.ts`
- `ui/src/ui/views/skills-grouping.ts`
- `ui/src/agents/tool-catalog.ts`

Scope:

- Agent list and empty states: `Agents`, `No agents found`,
  `Select an agent`.
- Agent tabs: `Overview`, `Files`, `Tools`, `Skills`, `Channels`,
  `Cron Jobs`.
- Agent overview labels: `Workspace`, `Primary Model`, `Identity Name`,
  `Default`, `Identity Emoji`, `Skills Filter`, `Model Selection`,
  `Fallbacks`.
- Status/files panels: `Agent Context`, `Channels`, `Scheduler`,
  `Agent Cron Jobs`, `Core Files`, `Content`, empty states, refresh/save
  button states.
- Tool access panel: `Tool Access`, `Profile`, `Source`, `Status`,
  `Quick Presets`, preset names.
- Skill panel in agent view: `This agent uses a custom skill allowlist`,
  filters, empty states.
- Tool catalog category labels and descriptions.

Notes:

- Tool names, schema IDs, model IDs, provider names, and paths should stay as
  data values unless they are display-only labels.

## Batch 4: Skills

Primary files:

- `ui/src/ui/views/skills.ts`
- `ui/src/ui/views/skills-grouping.ts`
- `ui/src/ui/controllers/skills.ts`

Scope:

- Skills page title/subtitle, filter label, search placeholder, empty state.
- Enable/disable buttons and API key label.
- Skill grouping labels: `Workspace Skills`, `Installed Skills`,
  `Extra Skills`, `Other Skills`.
- Toast/status messages: `Skill enabled`, `Skill disabled`, `API key saved`.

Notes:

- Skill names, source IDs, and package IDs should remain untranslated.

## Batch 5: Nodes And Exec Approvals

Primary files:

- `ui/src/ui/views/nodes.ts`
- `ui/src/ui/views/nodes-exec-approvals.ts`
- `ui/src/ui/views/exec-approval.ts`
- `ui/src/ui/controllers/devices.ts`
- `ui/src/ui/controllers/exec-approvals.ts`

Scope:

- Nodes/devices titles, subtitles, empty states, token labels.
- Device pairing sections: `Pending`, `Paired`, prompts/confirmations for
  reject/revoke/new token.
- Exec node binding labels and help text.
- Exec approvals page: `Exec approvals`, `Target`, `Host`, `Gateway`, `Node`,
  `Scope`, `Security`, `Ask`, `Ask fallback`, `Allowlist`, `Pattern`,
  buttons and empty states.
- Exec approval modal: title, subtitle, action buttons, status labels.

Notes:

- Command names, node IDs, device IDs, roles, glob patterns, and config paths
  should remain data values.

## Batch 6: Cron

Primary files:

- `ui/src/ui/views/cron.ts`
- `ui/src/ui/controllers/cron.ts`

Scope:

- Summary labels: `Enabled`, `Jobs`, `Next wake`.
- Jobs filters and sort controls.
- Run history filters and empty states.
- Job form sections: `Basics`, `Schedule`, `Execution`, `Delivery`,
  `Advanced`.
- Job form fields, placeholders, option labels, help text.
- Run/job detail labels and action links.
- Validation errors from `controllers/cron.ts`.

Notes:

- Cron expressions, timezone names, model IDs, webhook URLs, payload text, and
  enum values sent to the API should remain raw values. Translate only labels.

## Batch 7: Config UI

Primary files:

- `ui/src/ui/views/config.ts`
- `ui/src/ui/views/config-form.render.ts`
- `ui/src/ui/views/config-form.node.ts`
- `ui/src/ui/views/config-search.ts`
- `ui/src/ui/views/config-form.shared.ts`

Scope:

- Config sidebar and search UI: `Settings`, `Search settings`, `Tag filters`,
  `All Settings`, `No changes`, `Raw JSON5`.
- Config section labels/descriptions from `config-form.render.ts`.
- Form renderer messages: schema unavailable, unsupported schema/node/array,
  select placeholder, reset/remove titles, custom entries, key/value
  placeholders.
- Buttons: load/reload/save/apply/update and busy states.

Notes:

- Config keys, JSON paths, schema names, enum values, and JSON examples should
  usually stay untranslated.

## Batch 8: Usage

Primary files:

- `ui/src/ui/views/usage.ts`
- `ui/src/ui/views/usage-render-overview.ts`
- `ui/src/ui/views/usage-render-details.ts`
- `ui/src/ui/views/usage-metrics.ts`
- `ui/src/ui/views/usage-query.ts`

Scope:

- Usage page header, filters, date controls, timezone labels, refresh hints.
- Query placeholder and remove/clear filter controls.
- Usage overview cards, summaries, chart titles, tooltips, no-data states.
- Sessions panel labels, sort options, copy button, empty states.
- Session details: summary cards, timeline, token/context charts,
  conversation filters, role labels, close/reset/expand/collapse controls.
- Activity metrics labels: `Activity by Time`, `Day of Week`, `Hours`,
  `Midnight`, `Noon`, timeline empty states.

Notes:

- Model names, provider names, token counts, costs, query syntax examples, and
  session keys should remain data values.

## Batch 9: Gateway URL Confirmation And Device/Auth Messages

Primary files:

- `ui/src/ui/views/gateway-url-confirmation.ts`
- `ui/src/ui/device-auth.ts`
- `ui/src/ui/device-identity.ts`
- `ui/src/ui/gateway-events.ts`
- `ui/src/ui/gateway.ts`
- `ui/src/gateway/protocol/connect-error-details.ts`

Scope:

- Gateway URL confirmation modal title/subtitle/actions.
- Device/auth user-facing error messages.
- Gateway event user-visible status messages.
- Connection error detail messages if they are displayed directly in UI.

Notes:

- Error codes, method names, URLs, and protocol constants should remain raw.
- Some errors may be developer diagnostics; only translate user-facing strings.

## Batch 10: Remaining Controllers And Misc UI

Primary files:

- `ui/src/ui/controllers/chat.ts`
- `ui/src/ui/controllers/channels.ts`
- `ui/src/ui/controllers/presence.ts`
- `ui/src/ui/app-channels.ts`
- `ui/src/ui/app-chat.ts`
- `ui/src/ui/app-settings.ts`
- `ui/src/ui/app-scroll.ts`
- `ui/src/ui/app-gateway.ts`
- `ui/src/ui/app-polling.ts`
- `ui/src/ui/assistant-identity.ts`
- `ui/src/auto-reply/reply/strip-inbound-meta.ts`
- `ui/src/shared/chat-envelope.ts`
- `ui/src/infra/format-time/format-relative.ts`

Scope:

- Any `alert`, `confirm`, `prompt`, toast/status, or visible fallback message.
- Any labels returned from helpers and displayed in views.

Notes:

- `Assistant` default name may remain as product-facing default, or be
  translated if it is purely a generic role label. Decide consistently.
- Metadata stripping helpers and shared envelopes may not need i18n unless their
  strings render directly in the UI.

## Usually Do Not Translate

Files or strings that are mostly technical, styling, data, or test fixtures:

- `ui/src/styles/**/*.css`
- `ui/src/ui/data/moonshot-kimi-k2.ts`
- `ui/src/ui/tool-display.json`
- `ui/src/ui/icons.ts`
- `ui/src/ui/types.ts`
- `ui/src/ui/ui-types.ts`
- `ui/src/ui/types/**/*.ts`
- Keyboard names such as `Enter`, `Shift`, `Escape`.
- HTTP methods such as `GET`, `POST`.
- Brand/product/channel names such as `RavBot`, `WhatsApp`, `Telegram`,
  `Discord`, `Slack`, `Signal`, `iMessage`, `Google Chat`, `Nostr`.
- Protocol names and standards such as `NIP-04`, `NIP-05`, `LUD-16`, `JSONL`,
  `JSON5`, `WebSocket`, `IANA`, `Tailscale Serve`.
- API/config keys, enum values, IDs, paths, URLs, model names, provider names,
  tokens, public keys, session keys, and command examples.

## Current Hotspot Scan

Recent scan still reported visible hardcoded strings in:

- `ui/src/ui/app-render.helpers.ts`
- `ui/src/ui/chat/copy-as-markdown.ts`
- `ui/src/ui/chat/grouped-render.ts`
- `ui/src/ui/chat/tool-cards.ts`
- `ui/src/ui/views/agents.ts`
- `ui/src/ui/views/agents-panels-status-files.ts`
- `ui/src/ui/views/agents-panels-tools-skills.ts`
- `ui/src/ui/views/agents-utils.ts`
- `ui/src/ui/views/channels.config.ts`
- `ui/src/ui/views/channels.nostr.ts`
- `ui/src/ui/views/channels.nostr-profile-form.ts`
- `ui/src/ui/views/config.ts`
- `ui/src/ui/views/config-form.node.ts`
- `ui/src/ui/views/config-form.render.ts`
- `ui/src/ui/views/cron.ts`
- `ui/src/ui/views/exec-approval.ts`
- `ui/src/ui/views/gateway-url-confirmation.ts`
- `ui/src/ui/views/markdown-sidebar.ts`
- `ui/src/ui/views/nodes.ts`
- `ui/src/ui/views/nodes-exec-approvals.ts`
- `ui/src/ui/views/skills.ts`
- `ui/src/ui/views/skills-grouping.ts`
- `ui/src/ui/views/usage.ts`
- `ui/src/ui/views/usage-metrics.ts`
- `ui/src/ui/views/usage-render-details.ts`
- `ui/src/ui/views/usage-render-overview.ts`
- `ui/src/ui/controllers/cron.ts`
- `ui/src/ui/controllers/devices.ts`
- `ui/src/ui/controllers/sessions.ts`
- `ui/src/ui/controllers/skills.ts`
- `ui/src/agents/tool-catalog.ts`

