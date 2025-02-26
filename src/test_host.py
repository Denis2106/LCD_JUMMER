import serial
import time

def calc_crc(s):
    res = 0
    for c in s:
        res ^= ord(c)

    return res % 256


def send(**data):
    pairs = []
    for k, v in data.items():
        pairs.append(f'{k}:{v}')

    res = ','.join(pairs)
    res = '{' + res + '}'
    res += chr( calc_crc(res) )
    port.write(res.encode())



port = serial.Serial('COM22', 115200)

send(info='ping from host')

send(cmd='reset')

time.sleep(1)
req = port.read_all()

print(f'Request: {req}')

send(mode=1, freq_min=700, freq_max=750)
send(mode=2, freq_min=750, freq_max=800)
send(mode=3, freq_min=800, freq_max=850)

buf = ''

while True:
    req = port.read_all()
    if not req: continue

    buf += req.decode()
    if '}' in buf:
        print(f'Request: {buf}')
        buf = ''

