# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import sys
sys.path.insert(0, os.path.abspath('..'))

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
]

templates_path = ['_templates']
exclude_patterns = []

# Localization
language = 'en'

# Zamezí kolizím názvů sekcí mezi soubory
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

# Header and small theme tweaks
html_title = "Statistics - Documentation"
html_theme_options = {
    "collapse_navigation": False,
    "navigation_depth": 3,
    "style_external_links": True,
}

# Extensions - TODO
todo_include_todos = True
