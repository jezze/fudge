include packages/base/rules.mk

packages: packages-base

packages-clean: packages-base-clean
	@rm -rf image/bin

