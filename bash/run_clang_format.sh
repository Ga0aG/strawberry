#!/bin/bash

find . -regextype egrep -regex ".*\.(c|cc|h|hh|cpp|hpp)$" -not -path '*/install/*' \
  -not -path '*/build/*' -not -path '*/log/*' -not -path '*/deps/*' | xargs clang-format-7 -i
