
COMMIT_HASH := $Format:%h$
GN_VERSION := 0.3.2
export GN_VERSION := $(if $(filter-out %:%h,$(COMMIT_HASH)),$(GN_VERSION)-$(COMMIT_HASH))

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
