#coding=utf-8

# just grab some sample data here for Training
#
import sys
import json
import urllib, urllib2
import jieba

default_server="http://192.168.1.108:9002"

def _write_data_into_file(fname, fdata):
    output = open(fname,'w')
    seg_list = jieba.cut(fdata, cut_all=True)
    txt = u' '.join(seg_list).encode('utf-8')
    output.write(txt)
    output.close()
    return 0

def _extract_plain_text(cat, article_id):

    requrl = '%s/info/gettext/?category=%d&id=%d' %(default_server, cat, article_id)
    req = urllib.urlopen(requrl)
    js_data = json.loads(req.read())
    return js_data['data']

def store_text(cat, response_text, compare = 0):
    idx = 0
    name = ''
    fulldata = '' # include title + body

    for it in response_text:
        print it['id']
        idx += 1
        if compare == 0:
            name = 'data/%d_%d.txt' %(cat, idx)
        else:
            name = 'compare_data/%d_%d.txt' %(cat, idx)

        fulldata = '%s\n%s' %(it['title'], _extract_plain_text(cat, it['id']))
        _write_data_into_file(name, fulldata)

    return 0

def get_all_news(category):
    param = ({'utype':2,'uid':'imei','category':category,'offset':0, 'count':100})
    req_url = default_server + '/info/'
    print 'the req url is %s' %(req_url)
    client = urllib2.Request(req_url)
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
    response = opener.open(client, json.JSONEncoder().encode(param))
    js_data = json.loads(response.read())
    store_text(category, js_data['post'])
    return 0

def prepare_compare_data():
    param = ({'utype':2,'uid':'imei','category':1,'offset':102, 'count':4})
    req_url = default_server + '/info/'
    print 'the req url is %s' %(req_url)
    client = urllib2.Request(req_url)
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
    response = opener.open(client, json.JSONEncoder().encode(param))
    js_data = json.loads(response.read())
    store_text(1, js_data['post'], 1) # 1 means store into a compare data dir

    param = ({'utype':2,'uid':'imei','category':2,'offset':102, 'count':4})
    req_url = default_server + '/info/'
    print 'the req url is %s' %(req_url)
    client = urllib2.Request(req_url)
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
    response = opener.open(client, json.JSONEncoder().encode(param))
    js_data = json.loads(response.read())
    store_text(2, js_data['post'], 1) # 1 means store into a compare data dir

    param = ({'utype':2,'uid':'imei','category':3,'offset':102, 'count':4})
    req_url = default_server + '/info/'
    print 'the req url is %s' %(req_url)
    client = urllib2.Request(req_url)
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
    response = opener.open(client, json.JSONEncoder().encode(param))
    js_data = json.loads(response.read())
    store_text(3, js_data['post'], 1) # 1 means store into a compare data dir

    param = ({'utype':2,'uid':'imei','category':4,'offset':102, 'count':4})
    req_url = default_server + '/info/'
    print 'the req url is %s' %(req_url)
    client = urllib2.Request(req_url)
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
    response = opener.open(client, json.JSONEncoder().encode(param))
    js_data = json.loads(response.read())
    store_text(4, js_data['post'], 1) # 1 means store into a compare data dir
    return 0

## Main Entry:
#alltypes = [1, 2, 3, 4]
#for i in alltypes:
#    get_all_news(i)
# prepare some new data...
prepare_compare_data()
