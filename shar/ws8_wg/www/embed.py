with open('index.html', 'rb') as html: html = html.read()
with open('index.js', 'rb') as js: js = js.read()
with open('www.h', 'wt') as out:
    out.write('#pragma once\n\n')

    out.write(f'const int index_html_len = {len(html)};\n')
    out.write('const unsigned char index_html[] = { ')
    out.write(', '.join([f'{b}' for b in html]))
    out.write(' };\n\n')

    out.write(f'const int index_js_len = {len(js)};\n')
    out.write('const unsigned char index_js[] = { ')
    out.write(', '.join([f'{b}' for b in js]))
    out.write(' };\n\n')
