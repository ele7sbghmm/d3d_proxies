import frida, sys, time

TARGET = "Simpsons.exe"

SCRIPT = """
    var d3d8 = Process.findModuleByName('d3d8.dll');
    if (d3d8) {
        send('d3d8.dll loaded, hooking...');
        
        var exports = d3d8.enumerateExports();
        exports.forEach(function(exp) {
            send('export: ' + exp.name + 'address: ' + exp.address);
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
"""

def on_message(message, data):
    print(message)

print(f"Waiting for {TARGET}...")

device = frida.get_local_device()

while True:
    try:
        for process in device.enumerate_processes():
            if process.name.lower() == TARGET.lower():
                print(f"Found {TARGET} pid:{process.pid}, attaching...")
                session = device.attach(process.pid)
                script = session.create_script(SCRIPT)
                script.on('message', on_message)
                script.load()
                print("Hooked, waiting for calls...")
                sys.stdin.read()
    except frida.ProcessNotFoundError:
        pass
    except Exception as e:
        print(e)
    
    time.sleep(0.1)
