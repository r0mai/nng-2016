#!/usr/bin/env bash

COLUMNS=$(tput cols)
LINES=$(tput lines)

./build/cat 2>/dev/null | gnuplot -persist \
  -e "set terminal dumb $COLUMNS $LINES" \
  -e "set xlabel 'Number of radioactive samples'" \
  -e "set offset graph 0, graph 0.1" \
  -e "unset key" \
  -e "plot '-' with yerrorbars"
