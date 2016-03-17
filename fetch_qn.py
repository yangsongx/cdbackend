#!/usr/bin/env python
# This is a file try traverse all file and try remove obsoletes
# to save money for the company
import sys
import time
import urllib2
from qiniu import Auth, BucketManager

access_key = '5haoQZguw4iGPnjUuJnhOGufZMjrQnuSdySzGboj'
secret_key = 'OADMEtVegAXAhCJBhRSXXeEd_YRYzEPyHwzJDs95'

bucket_name = 'caredear-cloud'

def get_qiniu(q, key, num, suffix):
    # below are testing code...
    base_url = 'http://caredear-cloud.qiniudn.com/%s' % (key)
    private_url = q.private_download_url(base_url, expires=3600)
    print(private_url)
    req = urllib2.Request(private_url)
    fmt = '/tmp/my/%d.%s' %(num, suffix)
    f = open(fmt, 'wb')
    data_lines = urllib2.urlopen(req).readlines()
    for data in data_lines:
        f.write(data)
    f.close()
    return 0

def _extrace_img_type(imgs):
    t = ''
    s = imgs.split('/')
    print 'the type:%s' %(s[1])
    t = s[1]
    return t

def get_all_images(q, fn):
    count = 0
    f = open(fn, 'r')
    line = f.readline()
    while line:
        print 'the lists: %s' %(line.strip())
        s = line.strip()
        fields = s.split()
        print 'key: %s %s %s' %(fields[0], fields[1], _extrace_img_type(fields[1]))
        get_qiniu(q, fields[0], count, _extrace_img_type(fields[1]))
        count += 1
        line = f.readline()
    f.close()
    return 0

#put test code for verification
def _pure_test():
    a='14557161648855572'
    print a[:-7]
    mark = int(a[:-7])
    cur = time.time()
    print 'the delta = %d sec, and %d Day' %((cur - mark), ((cur - mark)/(24*60*60)))
    return 0

# set 1 means within the @delta time
def select_img_based_on_date(puttime, delta):
    selected = 0

    int_put = int(puttime)
    cur = time.time()
    if (cur - int_put) <= delta:
        print 'Bingo~~'
        selected = 1

    return selected 

# function from SDK doc
def list_all(q, bucket_name, limit, bucket = None, prefix=None):
    FD_ALL = open('/tmp/all.list', 'w')
    FD = open('/tmp/the.list', 'w+')

    if bucket is None:
        bucket = BucketManager(q)
    marker = None
    eof = False
    while eof is False:
        ret, eof, info = bucket.list(bucket_name, prefix=prefix, marker=marker, limit=limit)
        marker = ret.get('marker', None)
        print '-->marker:%s' %(marker)
        for item in ret['items']:
            print( 'the item is:')
            print item
            # item['putTime'] truncate lower-7 digit will get Unix time
            the_unixtime = str(item['putTime'])
            sec_time = str(the_unixtime[:-7])
            print 'the time:%s, readable is:%s' %(sec_time, time.ctime(float(sec_time)))
            fmt = '%s %s\n' %(item['key'].encode("utf-8"), time.ctime(float(sec_time)))
            FD_ALL.writelines(fmt)
            # FIXME - currently, select recently month-uploaded image
            if select_img_based_on_date(sec_time, 30*24*3600) == 1:
                print 'Well, selected it'
                if item['mimeType'].find('image') != -1:
                    print '\t GOOD'
                    fmt = '%s %s\n' %(item['key'].encode("utf-8"), item['mimeType'])
                    FD.writelines(fmt)
            pass

    FD.close()
    FD_ALL.close()
    return 0


#####################################################################################################

qn = Auth(access_key, secret_key)

###main start entry here...
if len(sys.argv) > 1:

    if sys.argv[1] == 'download':
        print 'You want to download the file...'
        get_all_images(qn, '/tmp/the.list')
        exit() 

    print 'You are set %s limit' %(sys.argv[1])
    limit = sys.argv[1]
else:
    limit = None

#FIXME - Be careful to call this API!!
#_pure_test()
list_all(qn, bucket_name, limit)
#_test_foo(qn, '10000207/1431566341185473812')
