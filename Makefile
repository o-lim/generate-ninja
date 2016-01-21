
.PHONY: all
all: gn_all gn_test

.PHONY: clean
clean:
	@rm -rf out out_bootstrap

.PHONY: gn_all
gn_all: gn gn_unittests

.PHONY: bootstrap
bootstrap out_bootstrap/gn: tools/gn/bootstrap/bootstrap.py
	@(cd tools/gn/bootstrap && python bootstrap.py -s --no-clean)

out/Release/build.ninja: out_bootstrap/gn
	@out_bootstrap/gn gen --args='is_debug=false' out/Release

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

out/Release/gn_unittests.pass: out/Release/gn_unittests
	@$<
	@touch $@

.PHONY: gn_test
gn_test: out/Release/gn_unittests.pass
