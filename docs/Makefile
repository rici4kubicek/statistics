# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SPHINXMULTIVERSION ?= sphinx-multiversion
SOURCEDIR     = source
BUILDDIR      = build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile clean multiversion

# Build documentation for multiple versions (tags and branches)
multiversion:
	@echo "Building documentation for multiple versions..."
	$(SPHINXMULTIVERSION) "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS)
	@echo "Injecting version selector into all versions..."
	@python3 inject_version_selector.py
	@echo "Creating root redirect to latest version..."
	@cp "$(SOURCEDIR)/_templates/index_redirect.html" "$(BUILDDIR)/html/index.html"
	@echo ""
	@echo "Build finished. The HTML pages for all versions are in $(BUILDDIR)/html"
	@echo "Open $(BUILDDIR)/html/index.html to view the documentation."

# Clean all build artifacts including multiversion builds
clean:
	rm -rf "$(BUILDDIR)"
	@echo "Build directory cleaned."

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
