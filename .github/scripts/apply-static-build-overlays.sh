#!/usr/bin/env bash
# Single source of truth for static-build cherry-picks used by release and validate workflows.
# Run from repository root after checking out the release tag (detached HEAD is fine).
set -euo pipefail

CATEGORY="${1:-}"
if [[ ${CATEGORY} != "before_or_equal_1_5_2" && ${CATEGORY} != "after_1_5_2" ]]; then
    echo "Usage: $0 before_or_equal_1_5_2|after_1_5_2" >&2
    exit 1
fi

ROOT="${GITHUB_WORKSPACE:-}"
if [[ -z ${ROOT} ]]; then
    ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
fi
if [[ -z ${ROOT} || ! -d "${ROOT}/.git" ]]; then
    echo "Could not determine repository root (set GITHUB_WORKSPACE or run inside a git clone)." >&2
    exit 1
fi
cd "${ROOT}"

git config user.name "github-actions[bot]"
git config user.email "github-actions[bot]@users.noreply.github.com"
git fetch origin main
# Not on main but required for cherry-pick objects
git fetch origin c8192e2311785a21894d7311f079b28caa541ca5

cherry_pick_prefer_commit() {
    local commit_sha="$1"
    if git cherry-pick "${commit_sha}"; then
        return 0
    fi

    echo "Cherry-pick ${commit_sha} conflicted. Retrying with '-X theirs' to prioritize cherry-pick changes."
    git cherry-pick --abort
    git cherry-pick -X theirs "${commit_sha}"
}

case "${CATEGORY}" in
before_or_equal_1_5_2)
    cherry_pick_prefer_commit b92492bb2232f0c1d502cfc0c6aede2f3169116b
    cherry_pick_prefer_commit 8ed41f2f8d59d63c09bb47ac8ad59d4f56c7d07c
    cherry_pick_prefer_commit 36c5e266dbf2bba0df39a7f789615fa4a2ecb8c3
    cherry_pick_prefer_commit c8192e2311785a21894d7311f079b28caa541ca5
    ;;
after_1_5_2)
    cherry_pick_prefer_commit c8192e2311785a21894d7311f079b28caa541ca5
    ;;
esac
