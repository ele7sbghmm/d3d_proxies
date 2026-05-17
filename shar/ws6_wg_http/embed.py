with open('index.js', 'rb') as js:
	js = js.read()

with open('./index_js.h', 'wt') as h:
	h.write(f'unsigned int INDEX_JS_LEN = 0x{len(js):x};\n')
	h.write('unsigned char INDEX_JS[] = {')
	h.write(','.join(str(hex(c)) for c in js))
	h.write('};\n')
