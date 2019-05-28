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
GN_VERSION := 0.4.1
export GN_VERSION := $(if $(filter-out %:%h %:%h$$,$(COMMIT_HASH)),$(GN_VERSION)$(VERSION_SUFFIX))

OUTDIR=out

.PHONY: all
all: gn_all gn_test $(OUTDIR)/.envvars
	@true

.PHONY: clean
clean:
	@rm -rf out

.PHONY: gn_all
gn_all: gn gn_unittests
	@true

$(OUTDIR)/bootstrap/build.ninja: build/gen.py
	@env -u CC -u CXX $< --out-path $(@D)

$(OUTDIR)/bootstrap/gn: $(OUTDIR)/bootstrap/build.ninja
	@ninja -C $(@D)
	@touch $@

.PHONY: bootstrap
bootstrap: $(OUTDIR)/bootstrap/gn
	@true

.PHONY: envvars
$(OUTDIR)/.envvars: envvars
	@echo "system=$(system)" >> $@.tmp
	@echo "CC=$(CC)" >> $@.tmp
	@echo "CXX=$(CXX)" >> $@.tmp
	@echo "DEPLOY=$(DEPLOY)" >> $@.tmp
	@diff -Nq $@ $@.tmp > /dev/null && rm -f $@.tmp || mv $@.tmp $@

$(OUTDIR)/Release/build.ninja: $(OUTDIR)/bootstrap/gn Makefile $(wildcard .git/HEAD) $(OUTDIR)/.envvars
	@$< gen --args='is_debug=false is_official_build=true' //$(@D)

.PHONY: build.ninja
build.ninja: $(OUTDIR)/Release/build.ninja
	@true

$(OUTDIR)/Release/gn $(OUTDIR)/Release/gn_unittests: build.ninja
	@ninja -C $(@D) $(@F)

.PHONY: gn
gn: $(OUTDIR)/Release/gn
	@true

.PHONY: gn_unittests
gn_unittests: $(OUTDIR)/Release/gn_unittests
	@true

$(OUTDIR)/boostrap/gn_unittests.pass: $(OUTDIR)/boostrap/gn_unittests
	@$<
	@touch $@

$(OUTDIR)/Release/gn_unittests.pass: $(OUTDIR)/Release/gn_unittests
	@$<
	@touch $@

.PHONY: gn_test
gn_test: $(OUTDIR)/Release/gn_unittests.pass
	@true
