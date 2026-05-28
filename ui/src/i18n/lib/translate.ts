import { en } from "../locales/en.ts";
import type { Locale, TranslationMap } from "./types.ts";

type Subscriber = (locale: Locale) => void;

export const SUPPORTED_LOCALES: ReadonlyArray<Locale> = ["en", "zh-CN", "zh-TW", "pt-BR"];

const LOCALE_LOADERS: Partial<Record<Locale, () => Promise<TranslationMap>>> = {
  "zh-CN": async () => (await import("../locales/zh-CN.ts")).zh_CN,
  "zh-TW": async () => (await import("../locales/zh-TW.ts")).zh_TW,
  "pt-BR": async () => (await import("../locales/pt-BR.ts")).pt_BR,
};

export function isSupportedLocale(value: string | null | undefined): value is Locale {
  return value !== null && value !== undefined && SUPPORTED_LOCALES.includes(value as Locale);
}

class I18nManager {
  private locale: Locale = "en";
  private translations: Record<Locale, TranslationMap> = { en } as Record<Locale, TranslationMap>;
  private subscribers: Set<Subscriber> = new Set();
  public ready: Promise<void>;

  constructor() {
    this.loadLocale();
    this.ready = this.ensureLocaleLoaded(this.locale).then((loaded) => {
      if (loaded) {
        this.notify();
      }
    });
  }

  private loadLocale() {
    const saved = globalThis.localStorage?.getItem("ravbot.i18n.locale");
    if (isSupportedLocale(saved)) {
      this.locale = saved;
    } else {
      const navLang = globalThis.navigator?.language ?? "en";
      if (navLang.startsWith("zh")) {
        this.locale = navLang === "zh-TW" || navLang === "zh-HK" ? "zh-TW" : "zh-CN";
      } else if (navLang.startsWith("pt")) {
        this.locale = "pt-BR";
      } else {
        this.locale = "en";
      }
    }
  }

  public getLocale(): Locale {
    return this.locale;
  }

  public async setLocale(locale: Locale) {
    const changed = this.locale !== locale;
    if (!changed && this.translations[locale]) {
      return;
    }

    const loaded = await this.ensureLocaleLoaded(locale);
    if (!loaded) {
      return;
    }

    this.locale = locale;
    globalThis.localStorage?.setItem("ravbot.i18n.locale", locale);
    this.notify();
  }

  public registerTranslation(locale: Locale, map: TranslationMap) {
    this.translations[locale] = map;
  }

  public subscribe(sub: Subscriber) {
    this.subscribers.add(sub);
    return () => this.subscribers.delete(sub);
  }

  private notify() {
    this.subscribers.forEach((sub) => sub(this.locale));
  }

  private async ensureLocaleLoaded(locale: Locale): Promise<boolean> {
    if (this.translations[locale]) {
      return true;
    }

    const load = LOCALE_LOADERS[locale];
    if (!load) {
      return false;
    }

    try {
      this.translations[locale] = await load();
      return true;
    } catch (e) {
      console.error(`Failed to load locale: ${locale}`, e);
      return false;
    }
  }

  public t(key: string, params?: Record<string, string>): string {
    const keys = key.split(".");
    let value: unknown = this.translations[this.locale] || this.translations["en"];

    for (const k of keys) {
      if (value && typeof value === "object") {
        value = (value as Record<string, unknown>)[k];
      } else {
        value = undefined;
        break;
      }
    }

    // Fallback to English
    if (value === undefined && this.locale !== "en") {
      value = this.translations["en"];
      for (const k of keys) {
        if (value && typeof value === "object") {
          value = (value as Record<string, unknown>)[k];
        } else {
          value = undefined;
          break;
        }
      }
    }

    if (typeof value !== "string") {
      return key;
    }

    if (params) {
      return value.replace(/\{(\w+)\}/g, (_, k) => params[k] || `{${k}}`);
    }

    return value;
  }
}

export const i18n = new I18nManager();
export const t = (key: string, params?: Record<string, string>) => i18n.t(key, params);
