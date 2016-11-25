#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PID=

cd "${REPOSITORY}"

isRebuildNeeded() {
  git remote update &>/dev/null
  diffcount=$(git rev-list HEAD...origin/master --count)
  if [ "$diffcount" -gt "0" ]; then
    echo "yes"
  else
    echo "no"
  fi
}

rebuild() {
  git checkout origin/master &>/dev/null
  make -j8 -C build &>/dev/null
}

restartBees() {
  kill "$PID"
  watch -n 1 "./build/bees 2>&1" &
  PID=$!
}



restartBees

while true; do
  if [ "$(isRebuildNeeded)" == "yes" ]; then
    echo "Rebuilding"
    rebuild
    restartBees
  fi
  sleep 1
done
