import frida, sys

def on_msg(msg,data): print(msg["msg"])
pid = frida.spawn(
    r"C:\Users\imw\Desktop\mi\shar\Simpsons.exe",
    cwd=r"C:\Users\imw\Desktop\mi\shar"
    )
session = frida.attach(pid)

script = session.create_script("""
    var d3d8 = Module.getExportByName('d3d8.dll', 'Direct3DCreate8');
    Interceptor.attach(d3d8, {
        onEnter(args) {
            send('Direct3DCreate8 called, SDKVersion: ' + args[0].toInt32());
        },  
        onLeave(retval) {
            send('Direct3DCreate8 returned IDirect3D8*: ' + retval);
        }
    });
""")

script.on('message', on_msg)
script.load()

frida.resume(pid)
sys.stdin.read()
