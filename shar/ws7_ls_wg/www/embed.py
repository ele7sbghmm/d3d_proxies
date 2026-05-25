with open('index.html', 'rb') as html:
    html = html.read()
with open('index.js', 'rb') as js:
    js = js.read()
with open('www.hpp', 'wt') as w:
    w.wreite('#pragma once\n')
    w.write(f'const int index_html_size = 0x{len(html):x};\n')
    w.write('const unsigned char index_html[] = { ')
    w.write(', '.join([f'0x{x:02x}' for x in html]))
    w.write(' };\n')
    
    w.write(f'const int index_js_size = 0x{len(js):x};\n')
    w.write('const unsigned char index_js[] = { ')
    w.write(', '.join([f'0x{x:02x}' for x in js]))
    w.write(' };\n')
