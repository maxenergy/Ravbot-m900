import { html, nothing } from "lit";
import { t } from "../../i18n/index.ts";
import { formatRelativeTimestamp } from "../format.ts";
import type { DiscordStatus } from "../types.ts";
import { renderChannelConfigSection } from "./channels.config.ts";
import { formatChannelBool } from "./channels.shared.ts";
import type { ChannelsProps } from "./channels.types.ts";

export function renderDiscordCard(params: {
  props: ChannelsProps;
  discord?: DiscordStatus | null;
  accountCountLabel: unknown;
}) {
  const { props, discord, accountCountLabel } = params;

  return html`
    <div class="card">
      <div class="card-title">Discord</div>
      <div class="card-sub">${t("channels.subtitles.bot")}</div>
      ${accountCountLabel}

      <div class="status-list" style="margin-top: 16px;">
        <div>
          <span class="label">${t("channels.status.configured")}</span>
          <span>${formatChannelBool(discord?.configured)}</span>
        </div>
        <div>
          <span class="label">${t("channels.status.running")}</span>
          <span>${formatChannelBool(discord?.running)}</span>
        </div>
        <div>
          <span class="label">${t("channels.status.lastStart")}</span>
          <span>${discord?.lastStartAt ? formatRelativeTimestamp(discord.lastStartAt) : t("common.na")}</span>
        </div>
        <div>
          <span class="label">${t("channels.status.lastProbe")}</span>
          <span>${discord?.lastProbeAt ? formatRelativeTimestamp(discord.lastProbeAt) : t("common.na")}</span>
        </div>
      </div>

      ${
        discord?.lastError
          ? html`<div class="callout danger" style="margin-top: 12px;">
            ${discord.lastError}
          </div>`
          : nothing
      }

      ${
        discord?.probe
          ? html`<div class="callout" style="margin-top: 12px;">
            ${t("channels.probe.result", { status: discord.probe.ok ? t("common.okLower") : t("common.failed") })} ·
            ${discord.probe.status ?? ""} ${discord.probe.error ?? ""}
          </div>`
          : nothing
      }

      ${renderChannelConfigSection({ channelId: "discord", props })}

      <div class="row" style="margin-top: 12px;">
        <button class="btn" @click=${() => props.onRefresh(true)}>
          ${t("channels.actions.probe")}
        </button>
      </div>
    </div>
  `;
}
