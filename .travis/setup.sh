#!/bin/bash

set -eufo pipefail

NINJA_VERSION="1.8.2"
CLANG_VERSION="6.0.1"

source .travis/platform.sh

mkdir -p $HOME/bin

if [ "$PLATFORM" == "macosx" ]; then
  NINJA_ZIP=ninja-mac.zip
elif [ "$PLATFORM" == "cygwin" ]; then
  NINJA_ZIP=ninja-win.zip
elif [ "$PLATFORM" == "windows" ]; then
  NINJA_ZIP=ninja-win.zip
elif [ "$PLATFORM" == "linux" ]; then
  NINJA_ZIP=ninja-linux.zip

  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
  sudo apt-get update -qq
  sudo apt-get install -qq gcc-5 g++-5 gcc-5-multilib g++-5-multilib libc6:i386 libstdc++6:i386 libc6-dev:i386 linux-libc-dev:i386
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 70 --slave /usr/bin/g++ g++ /usr/bin/g++-5
  sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30
  sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30
  sudo update-alternatives --set cc /usr/bin/gcc
  sudo update-alternatives --set c++ /usr/bin/g++
  sudo update-alternatives --set gcc /usr/bin/gcc-5

  curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
  sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main"
  sudo apt-get update -qq
  sudo apt-get install -qq clang-6.0 llvm-6.0 libc++-dev libc++abi-dev libc++-dev:i386 libc++abi-dev:i386
  sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-6.0 60 \
                           --slave /usr/bin/clang++ clang++ /usr/bin/clang++-6.0 \
                           --slave /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-6.0 \
                           --slave /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-6.0 \
                           --slave /usr/bin/llvm-as llvm-as /usr/bin/llvm-as-6.0 \
                           --slave /usr/bin/llvm-bcanalyzer llvm-bcanalyzer /usr/bin/llvm-bcanalyzer-6.0 \
                           --slave /usr/bin/llvm-cat llvm-cat /usr/bin/llvm-cat-6.0 \
                           --slave /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-6.0 \
                           --slave /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-6.0 \
                           --slave /usr/bin/llvm-c-test llvm-c-test /usr/bin/llvm-c-test-6.0 \
                           --slave /usr/bin/llvm-cxxdump llvm-cxxdump /usr/bin/llvm-cxxdump-6.0 \
                           --slave /usr/bin/llvm-cxxfilt llvm-cxxfilt /usr/bin/llvm-cxxfilt-6.0 \
                           --slave /usr/bin/llvm-diff llvm-diff /usr/bin/llvm-diff-6.0 \
                           --slave /usr/bin/llvm-dis llvm-dis /usr/bin/llvm-dis-6.0 \
                           --slave /usr/bin/llvm-dsymutil llvm-dsymutil /usr/bin/llvm-dsymutil-6.0 \
                           --slave /usr/bin/llvm-dwarfdump llvm-dwarfdump /usr/bin/llvm-dwarfdump-6.0 \
                           --slave /usr/bin/llvm-dwp llvm-dwp /usr/bin/llvm-dwp-6.0 \
                           --slave /usr/bin/llvm-extract llvm-extract /usr/bin/llvm-extract-6.0 \
                           --slave /usr/bin/llvm-lib llvm-lib /usr/bin/llvm-lib-6.0 \
                           --slave /usr/bin/llvm-link llvm-link /usr/bin/llvm-link-6.0 \
                           --slave /usr/bin/llvm-lto llvm-lto /usr/bin/llvm-lto-6.0 \
                           --slave /usr/bin/llvm-lto2 llvm-lto2 /usr/bin/llvm-lto2-6.0 \
                           --slave /usr/bin/llvm-mc llvm-mc /usr/bin/llvm-mc-6.0 \
                           --slave /usr/bin/llvm-mcmarkup llvm-mcmarkup /usr/bin/llvm-mcmarkup-6.0 \
                           --slave /usr/bin/llvm-modextract llvm-modextract /usr/bin/llvm-modextract-6.0 \
                           --slave /usr/bin/llvm-mt llvm-mt /usr/bin/llvm-mt-6.0 \
                           --slave /usr/bin/llvm-nm llvm-nm /usr/bin/llvm-nm-6.0 \
                           --slave /usr/bin/llvm-objcopy llvm-objcopy /usr/bin/llvm-objcopy-6.0 \
                           --slave /usr/bin/llvm-objdump llvm-objdump /usr/bin/llvm-objdump-6.0 \
                           --slave /usr/bin/llvm-opt-report llvm-opt-report /usr/bin/llvm-opt-report-6.0 \
                           --slave /usr/bin/llvm-profdata llvm-profdata /usr/bin/llvm-profdata-6.0 \
                           --slave /usr/bin/llvm-ranlib llvm-ranlib /usr/bin/llvm-ranlib-6.0 \
                           --slave /usr/bin/llvm-readelf llvm-readelf /usr/bin/llvm-readelf-6.0 \
                           --slave /usr/bin/llvm-readobj llvm-readobj /usr/bin/llvm-readobj-6.0 \
                           --slave /usr/bin/llvm-rtdyld llvm-rtdyld /usr/bin/llvm-rtdyld-6.0 \
                           --slave /usr/bin/llvm-size llvm-size /usr/bin/llvm-size-6.0 \
                           --slave /usr/bin/llvm-split llvm-split /usr/bin/llvm-split-6.0 \
                           --slave /usr/bin/llvm-stress llvm-stress /usr/bin/llvm-stress-6.0 \
                           --slave /usr/bin/llvm-strings llvm-strings /usr/bin/llvm-strings-6.0 \
                           --slave /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-6.0 \
                           --slave /usr/bin/llvm-tblgen llvm-tblgen /usr/bin/llvm-tblgen-6.0 \
                           --slave /usr/bin/llvm-xray llvm-xray /usr/bin/llvm-xray-6.0

  CLANG_INSTALL_PREFIX="/usr/local/$CLANG_VERSION"
  curl -SL http://releases.llvm.org/$CLANG_VERSION/clang+llvm-$CLANG_VERSION-x86_64-linux-gnu-ubuntu-16.04.tar.xz | sudo tar -xJ -C / --xform "s|clang+llvm-$CLANG_VERSION-x86_64-linux-gnu-ubuntu-16.04|$CLANG_INSTALL_PREFIX|"
  sudo update-alternatives --install /usr/bin/clang clang $CLANG_INSTALL_PREFIX/bin/clang 65 \
                          --slave /usr/bin/clang++ clang++ $CLANG_INSTALL_PREFIX/bin/clang++ \
                          --slave /usr/bin/clang-cpp clang-cpp $CLANG_INSTALL_PREFIX/bin/clang-cpp \
                          --slave /usr/bin/llvm-ar llvm-ar $CLANG_INSTALL_PREFIX/bin/llvm-ar \
                          --slave /usr/bin/llvm-as llvm-as $CLANG_INSTALL_PREFIX/bin/llvm-as \
                          --slave /usr/bin/llvm-bcanalyzer llvm-bcanalyzer $CLANG_INSTALL_PREFIX/bin/llvm-bcanalyzer \
                          --slave /usr/bin/llvm-cat llvm-cat $CLANG_INSTALL_PREFIX/bin/llvm-cat \
                          --slave /usr/bin/llvm-config llvm-config $CLANG_INSTALL_PREFIX/bin/llvm-config \
                          --slave /usr/bin/llvm-cov llvm-cov $CLANG_INSTALL_PREFIX/bin/llvm-cov \
                          --slave /usr/bin/llvm-c-test llvm-c-test $CLANG_INSTALL_PREFIX/bin/llvm-c-test \
                          --slave /usr/bin/llvm-cxxdump llvm-cxxdump $CLANG_INSTALL_PREFIX/bin/llvm-cxxdump \
                          --slave /usr/bin/llvm-cxxfilt llvm-cxxfilt $CLANG_INSTALL_PREFIX/bin/llvm-cxxfilt \
                          --slave /usr/bin/llvm-diff llvm-diff $CLANG_INSTALL_PREFIX/bin/llvm-diff \
                          --slave /usr/bin/llvm-dis llvm-dis $CLANG_INSTALL_PREFIX/bin/llvm-dis \
                          --slave /usr/bin/llvm-dsymutil llvm-dsymutil $CLANG_INSTALL_PREFIX/bin/llvm-dsymutil \
                          --slave /usr/bin/llvm-dwarfdump llvm-dwarfdump $CLANG_INSTALL_PREFIX/bin/llvm-dwarfdump \
                          --slave /usr/bin/llvm-dwp llvm-dwp $CLANG_INSTALL_PREFIX/bin/llvm-dwp \
                          --slave /usr/bin/llvm-extract llvm-extract $CLANG_INSTALL_PREFIX/bin/llvm-extract \
                          --slave /usr/bin/llvm-lib llvm-lib $CLANG_INSTALL_PREFIX/bin/llvm-lib \
                          --slave /usr/bin/llvm-link llvm-link $CLANG_INSTALL_PREFIX/bin/llvm-link \
                          --slave /usr/bin/llvm-lto llvm-lto $CLANG_INSTALL_PREFIX/bin/llvm-lto \
                          --slave /usr/bin/llvm-lto2 llvm-lto2 $CLANG_INSTALL_PREFIX/bin/llvm-lto2 \
                          --slave /usr/bin/llvm-mc llvm-mc $CLANG_INSTALL_PREFIX/bin/llvm-mc \
                          --slave /usr/bin/llvm-mcmarkup llvm-mcmarkup $CLANG_INSTALL_PREFIX/bin/llvm-mcmarkup \
                          --slave /usr/bin/llvm-modextract llvm-modextract $CLANG_INSTALL_PREFIX/bin/llvm-modextract \
                          --slave /usr/bin/llvm-mt llvm-mt $CLANG_INSTALL_PREFIX/bin/llvm-mt \
                          --slave /usr/bin/llvm-nm llvm-nm $CLANG_INSTALL_PREFIX/bin/llvm-nm \
                          --slave /usr/bin/llvm-objcopy llvm-objcopy $CLANG_INSTALL_PREFIX/bin/llvm-objcopy \
                          --slave /usr/bin/llvm-objdump llvm-objdump $CLANG_INSTALL_PREFIX/bin/llvm-objdump \
                          --slave /usr/bin/llvm-opt-report llvm-opt-report $CLANG_INSTALL_PREFIX/bin/llvm-opt-report \
                          --slave /usr/bin/llvm-profdata llvm-profdata $CLANG_INSTALL_PREFIX/bin/llvm-profdata \
                          --slave /usr/bin/llvm-ranlib llvm-ranlib $CLANG_INSTALL_PREFIX/bin/llvm-ranlib \
                          --slave /usr/bin/llvm-readelf llvm-readelf $CLANG_INSTALL_PREFIX/bin/llvm-readelf \
                          --slave /usr/bin/llvm-readobj llvm-readobj $CLANG_INSTALL_PREFIX/bin/llvm-readobj \
                          --slave /usr/bin/llvm-rtdyld llvm-rtdyld $CLANG_INSTALL_PREFIX/bin/llvm-rtdyld \
                          --slave /usr/bin/llvm-size llvm-size $CLANG_INSTALL_PREFIX/bin/llvm-size \
                          --slave /usr/bin/llvm-split llvm-split $CLANG_INSTALL_PREFIX/bin/llvm-split \
                          --slave /usr/bin/llvm-stress llvm-stress $CLANG_INSTALL_PREFIX/bin/llvm-stress \
                          --slave /usr/bin/llvm-strings llvm-strings $CLANG_INSTALL_PREFIX/bin/llvm-strings \
                          --slave /usr/bin/llvm-symbolizer llvm-symbolizer $CLANG_INSTALL_PREFIX/bin/llvm-symbolizer \
                          --slave /usr/bin/llvm-tblgen llvm-tblgen $CLANG_INSTALL_PREFIX/bin/llvm-tblgen \
                          --slave /usr/bin/llvm-xray llvm-xray $CLANG_INSTALL_PREFIX/bin/llvm-xray

  sudo update-alternatives --set clang $CLANG_INSTALL_PREFIX/bin/clang
fi

if [ -n "$NINJA_ZIP" ]; then
  wget https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/$NINJA_ZIP
  unzip -d $HOME/bin $NINJA_ZIP
  rm -f $NINJA_ZIP
fi

echo "$CC --version" && $CC --version
echo "$CXX --version" && $CXX --version
echo "ninja --version" && ninja --version
