import serial

modes = '''
{mode:0,freq_min:600,freq_max:650,sf=6|7}
{mode:1,freq_min:610,freq_max:660,sf=6|7|8}
{mode:2,freq_min:620,freq_max:670,sf=9}
{mode:3,freq_min:630,freq_max:680,sf=6|7|8|9}
'''

def calc_crc(msg:str) -> int:
    res = 0
    for ch in msg:
        res ^= ord(ch)

    return res % 256


sport = serial.Serial('COM3', 9600)

for msg in modes.split('\n'):
    sport.write(msg.encode())
    sport.write(calc_crc(msg))
    print(msg)


while True:
    msg = sport.read_all()
    if msg:
        print(msg)