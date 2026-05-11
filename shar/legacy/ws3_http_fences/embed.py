with open('index.js', 'rb') as f:
	data = f.read()

with open('index_js.h', 'wt') as f:
	f.write(f'unsigned int index_js_len = {len(data)};\n')
	f.write('unsigned char index_js[] = { ')
	f.write(','.join(f'0x{b:02x}' for b in data))
	f.write(' };\n')
