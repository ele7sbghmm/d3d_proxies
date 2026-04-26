import frida, sys, time

device = frida.get_local_device()
TARGET = "Simpsons.exe"

def on_message(message, data):
    print(message)

SCRIPT = """
    function hookD3D8() {
        var d3d8 = Process.findModuleByName('d3d8.dll');
        if (d3d8) {
            send('d3d8.dll found, hooking...');
            var exports = d3d8.enumerateExports();
            exports.forEach(function(exp) {
                if (exp.name === 'Direct3DCreate8') {
                    send('hooking Direct3DCreate8 at: ' + exp.address);
                    Interceptor.attach(exp.address, {
                        onEnter(args) {
                            send('Direct3DCreate8 called, SDKVersion: ' + args[0].toInt32());
                        },
                        onLeave(retval) {
                            send('Direct3DCreate8 returned: ' + retval);
                        }
                    });
                }
            });
        }
    }

    var kernel32 = Process.findModuleByName('kernel32.dll');
    var loadLibA = kernel32.findExportByName('LoadLibraryA');
    var loadLibW = kernel32.findExportByName('LoadLibraryW');

    // check if already loaded
    hookD3D8();

    Interceptor.attach(loadLibA, {
        onLeave(retval) {
            var d3d8 = Process.findModuleByName('d3d8.dll');
            if (d3d8) hookD3D8();
        }
    });

    Interceptor.attach(loadLibW, {
        onLeave(retval) {
            var d3d8 = Process.findModuleByName('d3d8.dll');
            if (d3d8) hookD3D8();
        }
    });
"""

print(f"Waitign for {TARGET}...")

hooked = False
while True:
    try:
        if not hooked:
            for process in device.enumerate_processes():
                if process.name.lower() == TARGET.lower():
                    print(f"Found {TARGET} pid:{process.pid}, attaching...")
                    session = device.attach(process.pid)
                    script = session.create_script(SCRIPT)
                    script.on('message', on_message)
                    script.load()
                    hooked = True
                    print('Hooked, waiting for calls...')
    except Exception as e:
        print(e)
    time.sleep(0.1)

sys.stdin.read()
