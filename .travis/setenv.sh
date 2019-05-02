#!/bin/bash


if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  export CC=clang
  export CXX=clang++
elif [ "$TRAVIS_OS_NAME" == "windows" ]; then
  export CC=cl.exe
  export CXX=cl.exe
elif [ "$TRAVIS_OS_NAME" == "linux" ]; then
  export CC=clang
  export CXX=clang++
fi

if [[ "$PATH" != "$HOME/bin:"* ]]; then
  PATH="$HOME/bin:$PATH"
fi

# We need to remove clang from the PATH so Travis will use our newly installed version of clang
export PATH=$(echo $PATH | sed 's|:/usr/local/clang-[0-9][0-9\.]*/bin:|:|g')

bash .travis/setup.sh
