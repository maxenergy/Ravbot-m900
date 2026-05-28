#!/usr/bin/env python3
# Copyright 2025 RavBot Contributors
# SPDX-License-Identifier: Apache-2.0

"""Small RavBot license issuing server.

Run on the authorization host, for example:

  RAVBOT_LICENSE_SECRET='Rtp@20080513' python3 scripts/license_server.py \
    --host 0.0.0.0 --port 9981 --data ./ravbot-license-records.json

The RavBot client prints URLs like:

  http://120.197.43.148:9981/?machineCode=<machine-code>
"""

from __future__ import annotations

import argparse
import datetime as _dt
import hashlib
import html
import json
import os
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import parse_qs, urlparse


DEFAULT_SECRET = "Rtp@20080513"


def authorization_code(machine_code: str, secret: str) -> str:
    payload = f"{machine_code.strip().upper()}|{secret}"
    return hashlib.sha256(payload.encode("utf-8")).hexdigest().upper()


def now_iso() -> str:
    return _dt.datetime.now(_dt.timezone.utc).astimezone().isoformat(timespec="seconds")


def load_records(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return []
    return data if isinstance(data, list) else []


def save_records(path: Path, records: list[dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(records, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    try:
        os.chmod(path, 0o600)
    except OSError:
        pass


def upsert_record(path: Path, record: dict[str, Any]) -> None:
    records = load_records(path)
    machine_code = record.get("machineCode", "")
    replaced = False
    for idx, existing in enumerate(records):
        if existing.get("machineCode") == machine_code:
            merged = dict(existing)
            merged.update(record)
            merged["updatedAt"] = now_iso()
            records[idx] = merged
            replaced = True
            break
    if not replaced:
        record["createdAt"] = now_iso()
        record["updatedAt"] = record["createdAt"]
        records.append(record)
    save_records(path, records)


def parse_json_body(handler: BaseHTTPRequestHandler) -> dict[str, Any]:
    length = int(handler.headers.get("Content-Length", "0") or "0")
    if length <= 0:
        return {}
    raw = handler.rfile.read(length)
    content_type = handler.headers.get("Content-Type", "")
    if "application/json" in content_type:
        try:
            data = json.loads(raw.decode("utf-8"))
            return data if isinstance(data, dict) else {}
        except Exception:
            return {}
    parsed = parse_qs(raw.decode("utf-8"))
    return {key: values[-1] if values else "" for key, values in parsed.items()}


class LicenseHandler(BaseHTTPRequestHandler):
    server: "LicenseHTTPServer"

    def log_message(self, fmt: str, *args: Any) -> None:
        if self.server.verbose:
            super().log_message(fmt, *args)

    def send_json(self, status: int, body: dict[str, Any]) -> None:
        data = json.dumps(body, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def send_html(self, status: int, body: str) -> None:
        data = body.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        params = parse_qs(parsed.query)
        if parsed.path == "/api/records":
            self.send_json(200, {"records": load_records(self.server.data_path)})
            return
        if parsed.path == "/api/issue":
            machine_code = (params.get("machineCode") or [""])[0].strip().upper()
            if not machine_code:
                self.send_json(400, {"error": "machineCode is required"})
                return
            code = authorization_code(machine_code, self.server.secret)
            record = {
                "machineCode": machine_code,
                "authorizationCode": code,
                "deviceInfo": {},
                "source": self.client_address[0],
            }
            upsert_record(self.server.data_path, record)
            self.send_json(200, record)
            return
        if parsed.path in ("", "/"):
            machine_code = (params.get("machineCode") or [""])[0].strip().upper()
            code = authorization_code(machine_code, self.server.secret) if machine_code else ""
            records = load_records(self.server.data_path)
            rows = "\n".join(
                "<tr>"
                f"<td><code>{html.escape(str(r.get('machineCode', '')))}</code></td>"
                f"<td><code>{html.escape(str(r.get('authorizationCode', '')))}</code></td>"
                f"<td>{html.escape(str(r.get('updatedAt', r.get('createdAt', ''))))}</td>"
                "</tr>"
                for r in reversed(records[-100:])
            )
            page = f"""<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>RavBot Authorization</title>
  <style>
    body {{ font-family: system-ui, sans-serif; max-width: 1100px; margin: 36px auto; padding: 0 24px; }}
    input, textarea {{ width: 100%; box-sizing: border-box; padding: 10px; font-family: ui-monospace, monospace; }}
    button {{ padding: 10px 14px; margin-top: 10px; }}
    code, pre {{ font-family: ui-monospace, monospace; }}
    pre {{ background: #f6f8fa; padding: 12px; overflow: auto; }}
    table {{ width: 100%; border-collapse: collapse; margin-top: 24px; }}
    th, td {{ border-bottom: 1px solid #ddd; padding: 8px; text-align: left; vertical-align: top; }}
  </style>
</head>
<body>
  <h1>RavBot Authorization</h1>
  <form method="get" action="/">
    <label>Machine code</label>
    <input name="machineCode" value="{html.escape(machine_code)}" autocomplete="off">
    <button type="submit">Generate authorization code</button>
  </form>
  <h2>Authorization code</h2>
  <pre>{html.escape(code)}</pre>
  <h2>Manual registration API</h2>
  <pre>POST /api/register
{{"machineCode":"...", "deviceInfo":{{...}}}}</pre>
  <h2>Recent registered devices</h2>
  <table>
    <thead><tr><th>Machine code</th><th>Authorization code</th><th>Updated</th></tr></thead>
    <tbody>{rows}</tbody>
  </table>
</body>
</html>"""
            if machine_code:
                upsert_record(
                    self.server.data_path,
                    {
                        "machineCode": machine_code,
                        "authorizationCode": code,
                        "deviceInfo": {},
                        "source": self.client_address[0],
                    },
                )
            self.send_html(200, page)
            return
        self.send_json(404, {"error": "not found"})

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path != "/api/register":
            self.send_json(404, {"error": "not found"})
            return
        body = parse_json_body(self)
        machine_code = str(body.get("machineCode", "")).strip().upper()
        if not machine_code:
            self.send_json(400, {"error": "machineCode is required"})
            return
        code = authorization_code(machine_code, self.server.secret)
        record = {
            "machineCode": machine_code,
            "authorizationCode": code,
            "deviceInfo": body.get("deviceInfo", {}),
            "source": self.client_address[0],
        }
        upsert_record(self.server.data_path, record)
        self.send_json(200, record)


class LicenseHTTPServer(ThreadingHTTPServer):
    def __init__(self, addr: tuple[str, int], handler: type[BaseHTTPRequestHandler], *, secret: str, data_path: Path, verbose: bool) -> None:
        super().__init__(addr, handler)
        self.secret = secret
        self.data_path = data_path
        self.verbose = verbose


def main() -> int:
    parser = argparse.ArgumentParser(description="RavBot authorization server")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=9981)
    parser.add_argument("--data", default="ravbot-license-records.json")
    parser.add_argument("--secret", default=os.environ.get("RAVBOT_LICENSE_SECRET", DEFAULT_SECRET))
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    server = LicenseHTTPServer(
        (args.host, args.port),
        LicenseHandler,
        secret=args.secret,
        data_path=Path(args.data),
        verbose=args.verbose,
    )
    print(f"RavBot authorization server listening on http://{args.host}:{args.port}")
    print(f"Records file: {Path(args.data).resolve()}")
    server.serve_forever()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
