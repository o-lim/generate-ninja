# Makefile to build gn bootstrap and release

ifdef system
# Export `system` if defined since it is used by BUILDCONFIG.gn
# Use system=i686-linux to build 32-bit binaries
export system
endif

ifdef DEPLOY
# Export `DEPLOY` if defined since it is used to indicate that we want
# to build for deployment. Use DEPLOY=1 to build binaries for deployment.
export DEPLOY
endif

# Export `CC` and `CXX` variables since they are used by gn files
# GN will be built using clang if CC=clang
export CC CXX

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
COMMA := ,

# Define the version of GN here with an optional commit hash appended
COMMIT_HASH := $Format:%h$
REF_NAMES := $Format:%D$
REF_NAMES := $(filter-out ->,$(subst $(COMMA),$(SPACE),$(subst :$(SPACE),:,$(REF_NAMES))))
VERSION_SUFFIX = $(if $(filter tag:v$(GN_VERSION),$(REF_NAMES)),,-$(COMMIT_HASH))
GN_VERSION := 0.3.2
export GN_VERSION := $(if $(filter-out %:%h %:%h$$,$(COMMIT_HASH)),$(GN_VERSION)$(VERSION_SUFFIX))

.PHONY: all
all: gn_all gn_test

.PHONY: clean
clean:
	@rm -rf out

.PHONY: gn_all
gn_all: gn gn_unittests

out/bootstrap/gn: build/gen.py
	@build/gen.py --out-path $(@D)
	@ninja -C $(@D)

.PHONY: bootstrap
bootstrap: out/bootstrap/gn
	@true

out/Release/build.ninja: out/bootstrap/gn Makefile
	@out/bootstrap/gn gen --args='is_debug=false is_official_build=true' //out/Release

.PHONY: build.ninja
build.ninja: out/Release/build.ninja

out/Release/gn: build.ninja
	@ninja -C out/Release gn

.PHONY: gn
gn: out/Release/gn

out/Release/gn_unittests: build.ninja
	@ninja -C out/Release gn_unittests

.PHONY: gn_unittests
gn_unittests: out/Release/gn_unittests

out/boostrap/gn_unittests.pass: out/boostrap/gn_unittests
	@$<
	@touch $@

out/Release/gn_unittests.pass: out/Release/gn_unittests
	@$<
	@touch $@

.PHONY: gn_test
gn_test: out/Release/gn_unittests.pass
