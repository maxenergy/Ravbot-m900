#!/bin/bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
INSTALL_SCRIPT="${REPO_ROOT}/scripts/install.sh"
TEST_ROOT="/tmp/ravbot-install-script-$$"
TEST_HOME="${TEST_ROOT}/home"
FAKE_BINARY="${TEST_ROOT}/ravbot"

cleanup() {
    rm -rf "${TEST_ROOT}"
}
trap cleanup EXIT

fail() {
    echo "[FAIL] $1"
    exit 1
}

pass() {
    echo "[PASS] $1"
}

mkdir -p "${TEST_HOME}"
cat > "${FAKE_BINARY}" <<'EOF'
#!/bin/sh
HOME_DIR="${HOME:-$PWD}"
case "${1:-}" in
  onboard)
    mkdir -p "${HOME_DIR}/.ravbot/agents/main/workspace"
    mkdir -p "${HOME_DIR}/.ravbot/agents/main/sessions"
    mkdir -p "${HOME_DIR}/.ravbot/logs"
    printf '{"agent":{},"gateway":{},"models":{}}\n' \
      > "${HOME_DIR}/.ravbot/ravbot.json"
    ;;
  gateway)
    if [ "${2:-}" = "install" ] && [ "$(uname -s)" = "Darwin" ]; then
      mkdir -p "${HOME_DIR}/Library/LaunchAgents"
      printf '<plist version="1.0"></plist>\n' \
        > "${HOME_DIR}/Library/LaunchAgents/com.ravbot.gateway.plist"
    fi
    ;;
  *)
    echo "ravbot test binary"
    ;;
esac
EOF
chmod +x "${FAKE_BINARY}"

OUT=$(
    HOME="${TEST_HOME}" \
    RAVBOT_INSTALL_TEST_MODE=1 \
    RAVBOT_INSTALL_SKIP_DEPS=1 \
    RAVBOT_INSTALL_PREFIX="${TEST_ROOT}/prefix" \
    bash "${INSTALL_SCRIPT}" --user --binary "${FAKE_BINARY}" 2>&1
) || fail "install.sh should support macOS-safe --user installs in test mode"

[[ -x "${TEST_HOME}/.ravbot/bin/ravbot" ]] \
    || fail "user install should place the binary under ~/.ravbot/bin"

[[ -f "${TEST_HOME}/.ravbot/ravbot.json" ]] \
    || fail "user install should create ravbot.json"

if [[ "$(uname -s)" == "Darwin" ]]; then
    [[ -f "${TEST_HOME}/Library/LaunchAgents/com.ravbot.gateway.plist" ]] \
        || fail "macOS install should create a launchd plist"
fi

echo "${OUT}" | grep -Eqi "success|installed" \
    || fail "installer output should report success"

pass "install.sh user-mode flow"
