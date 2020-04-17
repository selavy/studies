#!/usr/bin/env bash

die() {
    echo $1
    exit 1
}

for f in ./schemas/*.fbs; do
    flatc -c $f || die "failed to generate for $f"
done
