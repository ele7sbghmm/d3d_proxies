import * as fs from 'fs'
import * as ts from 'typescript'

const src = fs.readFileSync('index.ts', 'utf8')
const res = ts.transpileModule(src, {
    compilerOptions: {
        target: ts.ScriptTarget.ES2020,
    }
})

const js = Buffer.from(res.outputText)
const html = fs.readFileSync('index.html')

function toHex(buf: Buffer): string {
    return Array.from(buf).map(b => `0x${b.toString(16).padStart(2, '0')}`).join(', ')
}

const hpp = 
`#pragma once

const int index_html_size = ${html.length};
const unsigned char index_html[] = { ${toHex(html)} };

const int index_js_size = ${js.length};
const unsigned char index_js[] = { ${toHex(js)} };
`

fs.writeFileSync('../www.hpp', hpp)
