#!/usr/bin/env bash

while IFS= read -r line; do
    key="${line%%=*}"
    value="${line#*=}"

    git config --global "$key" "$value"
done < .devcontainer/scripts/gitconfig.export

# NOTE: devcontainer forces following properties as "global", thus pre-commit wrongly clones repositories
git config --global --unset remote.origin.url
git config --global --unset remote.origin.fetch
