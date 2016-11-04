#!/usr/bin/env bash

COLUMNS=$(tput cols)
LINES=$(tput lines)

./build/cat 2>/dev/null | gnuplot \
  -e "set terminal dumb $COLUMNS $LINES" \
  -e "set xlabel 'Number of radioactive samples'" \
  -e "unset key" \
  -e "plot '-' with yerrorbars"
