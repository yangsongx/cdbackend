import sys
import logging
from operator import itemgetter

logging.basicConfig(filename='streampy.log')

ret_dict = {}

try:
    for line in sys.stdin:
        line = line.strip()
        logging.info('[in reduce]now, got the line : %s' %(line) )

        key, val = line.split()
        count = int(val)
        ret_dict[key] = ret_dict.get(key, 0) + count # increment each one
    # Now try sort 
    sorted_ret_dict = sorted(ret_dict.items(), key=itemgetter(0))
    for k,v in sorted_ret_dict:
        print '%s\t%s' %(k,v)
except:
    logging.error('[reduce exception] %s || %s' \
            %(sys.exc_info()[0], sys.exc_info()[1]))
