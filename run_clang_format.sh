#!/bin/bash

if [[ $(which clang-format-10) ]]; then
    find . \( -name '*.cpp' -o -name '*.hpp' \) | xargs /usr/bin/clang-format-10 -i
else
    echo "clang-format-10 is required to format the code."
    echo "You can run 'sudo apt install clang-format-10' to get it."
fi