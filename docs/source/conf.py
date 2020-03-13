# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath('_extensions'))


# -- Project information -----------------------------------------------------

project = 'MDAL'
copyright = '2018-2020'
author = 'Peter Petrik, Martin Dobias and others'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'breathe',
    'configoptions',
    'redirects',
    'driverproperties'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = [ 'programs/options/*.rst' ]

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme_path = ['.']
html_theme = 'mdal_rtd'

html_context = {
  'display_github': True,
  'theme_vcs_pageview_mode': 'edit',
  'github_user': 'lutraconsulting',
  'github_repo': 'mdal',
  'github_version': '/master/docs/source/'
}

html_theme_options = {
    'canonical_url': 'https://mdal.xyz',
    'analytics_id': '',  #  Provided by Google in your dashboard
    'logo_only': True,
    'display_version': True,
    'prev_next_buttons_location': 'both',
    'style_external_links': False,
    #'vcs_pageview_mode': '',
    'style_nav_header_background': 'white',
    # Toc options
    'collapse_navigation': True,
    'sticky_navigation': True,
    #'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static']

# If true, links to the reST sources are added to the pages.
html_show_sourcelink = False

html_logo = '../images/LogoHorizontal_01_color_400x123.png'

# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).

author_peterp = 'Peter Petrik <zilolv@gmail.com>'

man_pages = [
    (
        'programs/mdalinfo',
        'mdalinfo',
        u'Lists various information about a MDAL supported dataset',
        [author_peterp],
        1
    ),
    (
        'programs/mdal_translate',
        'gdal_translate',
        u'Converts raster data between different formats.',
        [author_peterp],
        1
    ),
]


# latex

preamble = r"""
\ifdefined\DeclareUnicodeCharacter
  \DeclareUnicodeCharacter{2032}{$'$}% prime
\fi
"""

latex_elements = {
# The paper size ('letterpaper' or 'a4paper').
#'papersize': 'letterpaper',

# The font size ('10pt', '11pt' or '12pt').
#'pointsize': '10pt',

# Additional stuff for the LaTeX preamble.
'preamble': preamble,
'inputenc':'\\usepackage[utf8]{inputenc}\n\\usepackage{CJKutf8}\n\\usepackage{substitutefont}',
'babel':'\\usepackage[russian,main=english]{babel}\n\\selectlanguage{english}',
'fontenc':'\\usepackage[LGR,X2,T1]{fontenc}'

# Latex figure (float) alignment
#'figure_align': 'htbp',
}

latex_documents = [
    ('index_pdf', 'mdal.tex', project, author, 'manual'),
]

latex_toplevel_sectioning = 'chapter'

latex_logo = '../images/LogoHorizontal_01_color_400x123.png'

# If true, show URL addresses after external links.
#man_show_urls = False

# -- Breathe -------------------------------------------------

# Setup the breathe extension
breathe_projects = {
    "api": "../build/xml"
}
breathe_default_project = "api"

# Tell sphinx what the primary language being documented is.
primary_domain = 'c'

import redirects
redirect_files = redirects.gather_redirects()


