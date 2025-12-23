# Version Selector Documentation

The Statistics library documentation now includes a native version selector powered by `sphinx-multiversion`. This allows users to easily switch between different versions of the documentation.

## Features

- **Automatic Version Detection**: Automatically builds documentation for all git tags matching semantic versioning (`v*.*.*`) and main branches (`main`, `master`, `develop`)
- **Interactive Selector**: Click-to-open version selector at the bottom-left corner of each documentation page
- **Current Version Highlighting**: The currently viewed version is highlighted in the selector
- **Auto-redirect**: The root `index.html` automatically redirects to the latest version (main branch)

## Building Multi-Version Documentation

### Prerequisites

- Doxygen must be installed and available in PATH
- sphinx-multiversion must be installed: `pip install sphinx-multiversion`
- Git repository with tags and/or branches

### Build Commands

```bash
# From the repository root
cd docs

# 1. Generate Doxygen XML files (required for Breathe)
doxygen Doxyfile

# 2. Build documentation for all versions
make multiversion
```

The output will be in `docs/build/html/` with the following structure:

```
build/html/
├── index.html          # Root redirect to main
├── main/               # Main branch documentation
├── v1.0.0/            # Tag v1.0.0 documentation
├── v1.0.1/            # Tag v1.0.1 documentation
├── ...
└── v1.2.0/            # Latest tag documentation
```

## Configuration

### Version Patterns

Edit `docs/source/conf.py` to customize which versions are built:

```python
# Whitelist pattern for tags (Semantic versions only)
smv_tag_whitelist = r'^v\d+\.\d+\.\d+$'

# Whitelist pattern for branches (main and develop)
smv_branch_whitelist = r'^(main|master|develop)$'

# Whitelist pattern for remotes (origin only)
smv_remote_whitelist = r'^origin$'
```

### Customization

- **Template**: `docs/source/_templates/layout.html` - Controls the version selector HTML structure
- **Styling**: `docs/source/_static/version-selector.css` - CSS for version selector appearance
- **JavaScript**: `docs/source/_static/version-selector.js` - Interactive behavior (toggle open/close)
- **Redirect**: `docs/source/_templates/index_redirect.html` - Root redirect template

## How It Works

1. **sphinx-multiversion** scans the git repository for matching tags and branches
2. For each matched ref, it checks out that version and builds the documentation
3. All versions are built into separate directories under `build/html/`
4. The version selector template injects a list of available versions into each page
5. Users can click the selector to see and switch between all available versions

## Deployment Notes

When deploying the documentation:

1. Build the multi-version docs: `make multiversion`
2. Deploy the entire `build/html/` directory to your web server
3. The root `index.html` will redirect visitors to the main branch
4. Users can access any version directly via `/v1.0.0/`, `/main/`, etc.

## Troubleshooting

**Issue: No versions showing in selector**
- Ensure git tags match the whitelist pattern in `conf.py`
- Check that you're building from a git repository (not a tarball)
- Verify sphinx-multiversion is installed: `pip list | grep sphinx-multiversion`

**Issue: Doxygen errors during build**
- Run `doxygen Doxyfile` manually first to generate XML files
- Check that the `breathe_projects` path in `conf.py` points to the correct location

**Issue: Wrong version displayed**
- Check the template in `docs/source/_templates/layout.html`
- Verify `current_version` context variable is being passed correctly

## See Also

- [sphinx-multiversion documentation](https://holzhaus.github.io/sphinx-multiversion/)
- [Sphinx documentation](https://www.sphinx-doc.org/)
- [Breathe documentation](https://breathe.readthedocs.io/)
