#!/bin/sh

set -e  # exit if command fails
set -u  # error if variable not defined

# check dependencies
for cmd in curl tar grep uname; do
    command -v "$cmd" >/dev/null 2>&1 || {
        echo "Error: $cmd is not installed. Please install it and try again."
        exit 1
    }
done

# detect OS and architecture
arch=$(uname -m)
os=$(uname -s)

case $arch in
    x86_64) arch="amd64" ;;
    aarch64) arch="arm64" ;;
    *) echo "Unsupported architecture: $arch" && exit 1 ;;
esac

printf "Detected OS: %s\n" "$os"
printf "Detected architecture: %s\n" "$arch"

# ask user for version
printf "Select version (tag) to install (empty for latest): "
read -r version

if [ -z "$version" ]; then
    version="latest"
    url="https://api.github.com/repos/HermaDC/lord/releases/latest"
else
    url="https://api.github.com/repos/HermaDC/lord/releases/tags/$version"
fi

# get tag from GitHub API
tag=$(curl -fsSL "$url" | grep '"tag_name"' | cut -d '"' -f4) || {
    echo "Error getting version"
    exit 1
}

[ -z "$tag" ] && { echo "Could not determine tag"; exit 1; }

printf "Installing version: %s\n" "$tag"

# Download binary and then extract it
tmpfile=$(mktemp)

if ! curl -fsL -o "$tmpfile" "https://github.com/HermaDC/lord/releases/download/$tag/lord-$os-$arch.tar.gz"; then
    echo "Error downloading binary"
    rm -f "$tmpfile"
    exit 1
fi

if ! tar -xz -f "$tmpfile"; then
    echo "Error extracting files"
    rm -f "$tmpfile"
    exit 1
fi

rm -f "$tmpfile"

# docs
printf "Do you want to install documentation? (y/n): "
read -r install_docs

if [ "$install_docs" = "y" ]; then
    tmpfile=$(mktemp)

    if ! curl -fsL -o "$tmpfile" "https://github.com/HermaDC/lord/releases/download/$tag/lord-$tag-docs.tar.gz"; then
        echo "Error downloading documentation"
        rm -f "$tmpfile"
        exit 1
    fi

    tar -xz -f "$tmpfile" || {
        echo "Error extracting documentation"
        rm -f "$tmpfile"
        exit 1
    }

    rm -f "$tmpfile"
fi
