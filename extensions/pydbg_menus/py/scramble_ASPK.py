import os
import json
import sys

TOOL = '..\\bin\\SecurityCmd.exe'
TOOLCMD = 'scrambleaspk '
PRODUCT = '-product CDA'
PARAM_FILE = '..\..\extensions\pydbg_menus\py\user_defined_mod_seed_aspk.json'

def get_value(data, value):
    return data.get(value, None)

try:
    with open(PARAM_FILE,'r') as json_data:
        data = json.load(json_data)
        # Python 3: except FileNotFoundError:
except (IOError) as e:
        FILE_NOT_FOUND = 2
        if e.errno == FILE_NOT_FOUND:
            print("File not found")
        sys.exit(1)

mod = get_value(data, 'MOD')
seed = get_value(data, 'SEED')
aspk = get_value(data, 'ASPK')

cmd = ' '.join([TOOL, TOOLCMD, mod, seed, aspk, PRODUCT])
print('\nExecuting : {}\n'.format(cmd))
os.system(cmd)
