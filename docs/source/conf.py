# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import sys
import subprocess
sys.path.insert(0, os.path.abspath('..'))

# Run Doxygen to generate XML before Sphinx build
# This ensures each version has its own Doxygen XML
def run_doxygen(app, config):
    """Run Doxygen to generate XML files before building docs."""
    # Navigate up from source directory to docs directory
    source_dir = os.path.dirname(os.path.abspath(__file__))
    docs_dir = os.path.dirname(source_dir)
    doxyfile = os.path.join(docs_dir, 'Doxyfile')

    # Create build/doxygen directory if it doesn't exist
    doxygen_out = os.path.join(docs_dir, 'build', 'doxygen')
    os.makedirs(doxygen_out, exist_ok=True)

    if os.path.exists(doxyfile):
        print(f"Running Doxygen from {docs_dir}...")
        try:
            subprocess.run(['doxygen', doxyfile], cwd=docs_dir, check=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print("Doxygen completed successfully")
        except subprocess.CalledProcessError as e:
            print(f"Warning: Doxygen failed: {e}")
        except FileNotFoundError:
            print("Warning: Doxygen not found in PATH")
    else:
        print(f"Warning: Doxyfile not found at {doxyfile}")

def setup(app):
    app.connect('config-inited', run_doxygen)
    # Note: setup_version_selector creates directories but sphinx-multiversion
    # will check out the old version's files, overwriting our templates.
    # The real solution is to commit these files so they exist in all versions.

project = 'Statistics'
copyright = '2025, Richard Kubíček'
author = 'Richard Kubíček'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    'sphinx.ext.viewcode',
    'sphinx.ext.todo',
    'sphinx.ext.autosectionlabel',
    'sphinx_multiversion',
]

templates_path = ['_templates']
exclude_patterns = []

# Localization
language = 'en'

autosectionlabel_prefix_document = True

breathe_projects = {
    "Statistics": os.path.abspath('../build/doxygen/xml')
}
breathe_default_project = "Statistics"

# -- HTML --------------------------------------------------------------------

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
html_static_path = ['_static']
html_css_files = ['version-selector.css']
html_js_files = ['version-selector.js']

# Header and small theme tweaks
html_title = "Statistics - Documentation"
html_theme_options = {
    "collapse_navigation": False,
    "navigation_depth": 3,
    "style_external_links": True,
    "display_version": True,
}

# Add context for version selector
html_context = {
    'display_github': True,
    'github_user': 'rici4kubicek',
    'github_repo': 'statistics',
    'github_version': 'main',
    'conf_py_path': '/docs/source/',
}

# Extensions - TODO
todo_include_todos = True

# -- Sphinx-multiversion configuration ---------------------------------------

# Whitelist pattern for tags (Semantic versions only)
smv_tag_whitelist = r'^v\d+\.\d+\.\d+$'

# Whitelist pattern for branches (main and develop)
smv_branch_whitelist = r'^(main|master|develop)$'

# Whitelist pattern for remotes (origin only)
smv_remote_whitelist = r'^origin$'

# Pattern to exclude releases
smv_released_pattern = r'^tags/.*$'

# Output for multiversion builds
smv_outputdir_format = '{ref.name}'

# Latest version
smv_latest_version = 'main'

# Prefer tags over branches
smv_prefer_remote_refs = False

# Override templates and static files for all versions
# This ensures old versions get the version selector even if they didn't have it originally
import shutil

def copy_version_selector_files(app, exception):
    """Copy version selector templates/static files to all version builds."""
    if exception is not None:
        return

    # Get the main branch templates/static directory
    main_templates = os.path.join(app.srcdir, '_templates')
    main_static = os.path.join(app.srcdir, '_static')

    # Only proceed if we have the version selector files
    version_selector_css = os.path.join(main_static, 'version-selector.css')
    version_selector_js = os.path.join(main_static, 'version-selector.js')
    layout_template = os.path.join(main_templates, 'layout.html')

    if not all([os.path.exists(f) for f in [version_selector_css, version_selector_js, layout_template]]):
        print("Warning: Version selector files not found, skipping copy")
        return

# Setup hook to ensure version selector works across all versions
def setup_version_selector(app):
    """Ensure version selector templates are available for all built versions."""
    # This runs before each version build
    source_dir = os.path.dirname(os.path.abspath(__file__))

    # Ensure _templates directory exists
    templates_dir = os.path.join(source_dir, '_templates')
    os.makedirs(templates_dir, exist_ok=True)

    # Ensure _static directory exists
    static_dir = os.path.join(source_dir, '_static')
    os.makedirs(static_dir, exist_ok=True)
