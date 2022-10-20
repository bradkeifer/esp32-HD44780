from esp_docs.conf_docs import *  # noqa: F403,F401

languages = ['en', 'zh_CN']
#idf_targets = ['esp32', 'esp32s2', 'esp32s3', 'esp32c3', 'esp32h2', 'esp32c2', 'esp32c6']

extensions += ['sphinx_copybutton',
               'sphinxcontrib.wavedrom',
               # Needed as a trigger for running doxygen
               'esp_docs.esp_extensions.dummy_build_system',
               'esp_docs.esp_extensions.run_doxygen',
               ]

# link roles config
github_repo = 'bradkeifer/esp32-HD44780'

# context used by sphinx_idf_theme
html_context['github_user'] = 'bradkeifer'
html_context['github_repo'] = 'esp32-HD44780'

# Extra options required by sphinx_idf_theme
project_slug = 'lcd'

# Contains info used for constructing target and version selector
# Can also be hosted externally, see esp-idf for example
#versions_url = '_static/docs_version.js'

# Final PDF filename will contains target and version
pdf_file_prefix = u'lcd'
