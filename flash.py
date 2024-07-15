import subprocess

# Define the serial port and the binary file path
serial_port = 'COM5'  # Adjust this as per your system
binary_path = '.pio/build/esp32-c3-devkitm-1/firmware.bin'

# Command to run esptool.py
cmd = [
    'python',
    'esptool.py',
    '--chip', 'esp32',
    '--port', serial_port,
    '--baud', '921600',
    '--before', 'default_reset',
    '--after', 'hard_reset',
    'write_flash', '-z',
    '--flash_mode', 'dio',
    '--flash_freq', '40m',
    '--flash_size', 'detect',
    '0x10000', binary_path
]

# Execute the command
result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

# Check the result
if result.returncode == 0:
    print("Flashing successful!")
else:
    print("Error flashing:", result.stderr)
