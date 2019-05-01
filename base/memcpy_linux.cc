// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstddef>

extern "C" {

void * memcpy_glibc(void *, const void *, size_t);
#if defined(__x86_64__)
asm(".symver memcpy_glibc, memcpy@GLIBC_2.2.5");
#elif defined(__i386__) || defined(__i686__)
asm(".symver memcpy_glibc, memcpy@GLIBC_2.0");
#endif

void * __wrap_memcpy(void * dest, const void * src, size_t count) {
    return memcpy_glibc(dest, src, count);
}

} // extern "C"
