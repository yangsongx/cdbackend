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

def store_text(cat, response_text):
    idx = 0
    name = ''
    fulldata = '' # include title + body

    for it in response_text:
        print it['id']
        idx += 1
        name = 'data/%d_%d.txt' %(cat, idx)
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

## Main Entry:
alltypes = [1, 2, 3, 4]
for i in alltypes:
    get_all_news(i)
