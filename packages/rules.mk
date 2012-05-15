include packages/base/rules.mk
include packages/test/rules.mk
include packages/wm/rules.mk

packages: packages-base packages-test packages-wm

packages-clean: packages-base-clean packages-test-clean packages-wm-clean
	@rm -rf image/bin

