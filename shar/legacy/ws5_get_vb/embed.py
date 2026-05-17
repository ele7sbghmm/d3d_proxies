with open('index.js', 'rt') as js:
  js = js.read()

with open('index_js.h', 'wt') as embed:
	embed.write(f'unsigned int INDEX_JS_LEN = 0x{len(js):x};\n')
	embed.write('unsigned char INDEX_JS[] = { ')=
	f.write(','.join(f'0x{b:02x}' for b in js))
	embed.write(' };\n')
