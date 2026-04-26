import frida, sys

device = frida.get_local_device()

def on_message(message, data):
    print(message)

def on_child_added(child):
    print(f"child spawned: {child.pid} {child.pid}")
    session = device.attach(child.pid)
    script = session.create_script(SCRIPT)
    script.on('message', on_message)
    script.load()
    device.resume(child.pid)

SCRIPT = """
    var d3d8 = Process.findModuleByName('d3d8.dll');
    if (d3d8) {
        send('d3d8.dll already loaded');
    } else {
        send('d3d8.dll not loaded yet--too early');
    }
"""

pid = device.spawn(r"C:\Users\imw\Desktop\mi\donut\lml_1-27-1\lml_1-27-1.exe")
session = device.attach(pid)
device.enable_spawn_gating()
device.on('child-added', on_child_added)
device.resume(pid)

sys.stdin.read()
