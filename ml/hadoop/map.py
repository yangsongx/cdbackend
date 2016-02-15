import sys
import logging

logging.basicConfig(filename='streampy.log')

try:
    for line in sys.stdin:
        line = line.strip()
        logging.info('now, got the line : %s' %(line) )

        fields = line.split('\t')
        if fields[2] != None and fields[2] != '':
            print '%s\t1' %(fields[2])
except:
    logging.error('%s || %s' %(sys.exc_info()[0], sys.exc_info()[1]))
