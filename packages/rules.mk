include packages/base/rules.mk
include packages/net/rules.mk
include packages/test/rules.mk
include packages/wm/rules.mk

packages: packages-base packages-net packages-test packages-wm

packages-clean: packages-base-clean packages-net-clean packages-test-clean packages-wm-clean
	@rm -rf image/bin

