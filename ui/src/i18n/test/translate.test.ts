import { describe, it, expect, beforeEach, afterEach, vi } from "vitest";
import { i18n, t } from "../lib/translate.ts";
import { en } from "../locales/en.ts";
import { pt_BR } from "../locales/pt-BR.ts";
import { zh_CN } from "../locales/zh-CN.ts";
import { zh_TW } from "../locales/zh-TW.ts";
import type { TranslationMap } from "../lib/types.ts";

function installLocalStorage() {
  const store = new Map<string, string>();
  vi.stubGlobal("localStorage", {
    clear: () => store.clear(),
    getItem: (key: string) => store.get(key) ?? null,
    removeItem: (key: string) => store.delete(key),
    setItem: (key: string, value: string) => store.set(key, String(value)),
  });
}

describe("i18n", () => {
  beforeEach(async () => {
    installLocalStorage();
    localStorage.clear();
    // Reset to English
    await i18n.setLocale("en");
  });

  afterEach(() => {
    vi.unstubAllGlobals();
  });

  it("should return the key if translation is missing", () => {
    expect(t("non.existent.key")).toBe("non.existent.key");
  });

  it("should return the correct English translation", () => {
    expect(t("common.health")).toBe("Health");
  });

  it("should replace parameters correctly", () => {
    expect(t("overview.stats.cronNext", { time: "10:00" })).toBe("Next wake 10:00");
  });

  it("should fallback to English if key is missing in another locale", async () => {
    await i18n.setLocale("zh-CN");
    expect(t("non.existent.key")).toBe("non.existent.key");
  });

  it("should load Simplified Chinese translations", async () => {
    await i18n.setLocale("zh-CN");
    expect(t("common.health")).toBe("健康状况");
  });
});

describe("i18n startup locale", () => {
  afterEach(() => {
    vi.resetModules();
    vi.unstubAllGlobals();
  });

  it("loads a saved Chinese locale even when it is already selected at startup", async () => {
    installLocalStorage();
    localStorage.setItem("ravbot.i18n.locale", "zh-CN");
    vi.resetModules();

    const module = await import("../lib/translate.ts");
    await module.i18n.ready;

    expect(module.i18n.getLocale()).toBe("zh-CN");
    expect(module.t("common.health")).toBe("健康状况");
  });
});

function flattenKeys(map: TranslationMap, prefix = ""): string[] {
  return Object.entries(map).flatMap(([key, value]) => {
    const next = prefix ? `${prefix}.${key}` : key;
    return typeof value === "string" ? [next] : flattenKeys(value, next);
  });
}

describe("locale maps", () => {
  it("keeps translated locale keys aligned with English", () => {
    const expected = flattenKeys(en).toSorted();

    for (const [locale, map] of [
      ["zh-CN", zh_CN],
      ["zh-TW", zh_TW],
      ["pt-BR", pt_BR],
    ] as const) {
      expect(flattenKeys(map).toSorted(), locale).toEqual(expected);
    }
  });
});
