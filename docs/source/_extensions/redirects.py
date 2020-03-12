import os

# https://tech.signavio.com/2017/managing-sphinx-redirects


template="""<html>
  <head>
    <meta http-equiv="refresh" content="1; url=%s" />
    <script>
      window.location.href = "%s"
    </script>
  </head>
</html>"""


def gather_redirects():
    output = {}

    def fetch(path, d, prefix):
        files = os.listdir(path)
        for f in files:
            driver = f.split('.')[0]
            entry = {'%s_%s.html'% (prefix,driver) : os.path.join(path,driver)+'.html' }
            d.update(entry)

    fetch('./drivers', output, 'frmt')

    tools = [
        'mdalinfo',
    ]
    for utility in tools:
        output.update({ utility + '.html' : os.path.join('./programs/', utility) + '.html' })

    return output




from shutil import copyfile
# copy legacy redirects
def copy_legacy_redirects(app, docname): # Sphinx expects two arguments
    if app.builder.name == 'html':
        for key in app.config.redirect_files:
            src = key
            tgt = app.config.redirect_files[key]
            html = template % (tgt, tgt)
            with open(os.path.join(app.outdir, src), 'wb') as f:
                f.write(html.encode('utf-8'))
                f.close()



def setup(app):
    app.add_config_value('redirect_files', {}, 'html')
    app.connect('build-finished', copy_legacy_redirects)
    return { 'parallel_read_safe': False, 'parallel_write_safe': True }
