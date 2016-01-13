#coding=utf-8

# wpredict.py - weather predict
#
# which is predict the ZhiShu based on normal data


# [2016-01-13] i try first auto-learn the cloth-ZS field
#
import sys, os, re
import numpy
import jieba
import json
from sklearn.datasets import fetch_20newsgroups
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.naive_bayes import MultinomialNB
from sklearn.linear_model import SGDClassifier
from sklearn.pipeline import Pipeline
from sklearn import metrics

#static global definition, need map with @load_training_data()
ray_vals = ['最弱', '弱', '中等', '强']
ray_des = {
    '0':'辐射弱，涂擦SPF8-12防晒护肤品', #最弱
    '1':'辐射较弱，涂擦SPF12-15、PA+护肤品', #弱
    '2':'涂擦SPF大于15、PA+防晒护肤品', #中等
    '3':'涂擦SPF大于15、PA+防晒护肤品', #强
}

sick_vals = ['少发', '易发', '较易发', '极易发']
sick_des = {
    '0' : '无明显降温，感冒机率较低', #少发
    '0' : '昼夜温差大，易感冒', #易发
    '1' : '天较凉，增加衣服，注意防护', #较易发
    '2' : '天气寒冷，昼夜温差极大', #极易发
}

cloth_vals = ['热', '舒适', '较舒适', '冷', '较冷', '寒冷']
cloth_des = {
    '0' : '适合穿T恤、短薄外套等夏季服装', #热
    '1' : '建议穿长袖衬衫单裤等服装', #舒适
    '2' : '建议穿薄外套或牛仔裤等服装', #较舒适
    '3' : '建议着棉衣加羊毛衫等冬季服装', #冷
    '4' : '建议着厚外套加毛衣等服装', #较冷
    '5' : '建议着厚羽绒服等隆冬服装', #寒冷
}

car_vals = ['较适宜', '较不宜', '不宜']
car_des = {
    '0' : '无雨且风力较小，易保持清洁度', # 较适宜
    '0' : '风力较大，洗车后会蒙上灰尘', # 较不宜
    '1' : '有雨，雨水和泥水会弄脏爱车', # 不宜
}

zs1_type = {}
zs2_type = {}
zs3_type = {}
zs4_type = {}

###################################################
# This is a inner OBJ text for learning.txt
#
# for delta:
#      <5 : L  |  5<*<10 M | >10 H |
#
# for temp:
#      < -5     V (very cold)
#   -5 < * < 5  C (cold)
#    5 < * < 15 N (normal)
#   15 < * < 25 G (good)
#   25 < *      B (burn)
#
# for wind:
class TempObject:
    __private = 3

    # Ying, Qing, DuoYun, etc...
    overal_1 = ''
    overal_2 = ''

    wind_dir_1 = ''
    wind_frc_1 = ''
    wind_dir_2 = ''
    wind_frc_2 = ''
    wind_dir_3 = ''
    wind_frc_3 = ''

    temp_h1 = 'N'
    temp_l1 = 'N'
    temp_h2 = 'N'
    temp_l2 = 'N'

    delta1 = 'M'
    delta2 = 'M'
    delta3 = 'M'


    zs_cloth = 0

    def get_private(self):
        return self.__private

def predict_with_bayes(learn_data, learn_idx, check_data, check_idx):
    # create the pipline for Classifier
    bs_clf = Pipeline(
            [
                ('vect', CountVectorizer()),
                ('tfidf', TfidfTransformer()),
                ('clf', MultinomialNB()),
            ])
    bs_clf = bs_clf.fit(learn_data, learn_idx)
    bs_predict = bs_clf.predict(check_data)
    for i in range(0, len(check_idx)):
        print '(%d) [%d] ==> %s' % (bs_predict[i], check_idx[i], (check_data[i])[0:20] )

    print numpy.mean(bs_predict == check_idx)

    return 0
def predict_with_svm(learn_data, learn_idx, check_data, check_idx):
    # create the pipline for Classifier
    svm_clf = Pipeline(
            [
                ('vect', CountVectorizer()),
                ('tfidf', TfidfTransformer()),
                ('svmclf', SGDClassifier(loss='hinge', penalty='l2',
                                      alpha=1e-3, n_iter = 5, random_state=42)),
            ])
    svm_clf = svm_clf.fit(learn_data, learn_idx)
    svm_predict = svm_clf.predict(check_data)
    for i in range(0, len(check_idx)):
        print '(%d) [%d] ==> %s' % (svm_predict[i], check_idx[i], (check_data[i])[0:20] )

    print numpy.mean(svm_predict == check_idx)

    return 0

def _read_data_content(datadir, fnamelist):
    cat_idx = []
    cat_content = []


    for it in fnamelist:
        t = 0
        content = ''
        pattern = re.compile(r'(\d)\_\d*.txt')
        match = pattern.findall(it)
        t = int(match[0])
        fmt = '%s/%s' %(datadir, it)
        rdfile = open(fmt)
        while 1:
            line = rdfile.readline()
            if not line:
                break
            content += line
        cat_idx.append(t)
        cat_content.append(content)
        print '%d --> %s' %(t, content[0:10])

    return (cat_idx, cat_content)

def quick_insert_compare_data():
    content = ''
    outfile = open('raw.txt')
    while 1:
        line = outfile.readline()
        if not line:
            break
        content += line
    seg_list = jieba.cut(content, cut_all=True)
    txt = u' '.join(seg_list).encode('utf-8')
    secfile = open('c.txt', 'w')
    secfile.write(txt)
    secfile.close()
    return 0

# FIXME - this util is just extract all
# available ZS value types...
def _extract_all_types_(fn):
    fmt = 'data/%s' %(fn)
    print 'opening ... %s ...\n' %(fmt)

    fd = open(fmt, 'r')
    try:
        all_txt = fd.read()
        print 'get all text, they are:\n'
        js_data = json.loads(all_txt)
#print json.dumps(js_data['zs'], ensure_ascii=False)
        zs = js_data['zs']
        if zs.has_key('z1'):
            zs1 = zs['z1']

            if not zs1_type.has_key(zs1['v']):
                zs1_type[zs1['v']] = zs1['d']

        if zs.has_key('z2'):
            zs2 = zs['z2']
            if not zs2_type.has_key(zs2['v']):
                zs2_type[zs2['v']] = zs2['d']

        if zs.has_key('z3'):
            zs3 = zs['z3']
            if not zs3_type.has_key(zs3['v']):
                zs3_type[zs3['v']] = zs3['d']
        if zs.has_key('z4'):
            zs4 = zs['z4']
            if not zs4_type.has_key(zs4['v']):
                zs4_type[zs4['v']] = zs4['d']
    except:
           print '% || %s' %(sys.exe_info()[0], sys.exe_info()[1])
    finally:
        fd.close()

    return 0

#####################################################################
def _map_temp(temp):
    ret = 'N'

    if temp < -5:
        ret = 'V'
    elif temp >= -5 and temp < 5:
        ret = 'C'
    elif temp >=5 and temp < 15:
        ret = 'N'
    elif temp >=15 and temp < 25:
        ret = 'G'
    elif temp >=25:
        ret = 'B'

    return ret 
#####################################################################
def _map_delta_vals(delta):
    ret = 'L'

    if delta < 5:
        ret = 'L'
    elif delta >= 10:
        ret = 'H'
    else:
        ret = 'M'

    return ret 
#####################################################################
def _map_cloth_vals(val):
    i = 0
    en_val = u''.join(val).encode('utf-8')
    for it in cloth_vals:
        print 'it is %s, val is %s' %(it, en_val)
        
        if en_val == it:
            print 'Bingo'
            break;
        i += 1

    return i
#####################################################################
def _map_ray_vals(val):
    i = 0
    en_val = u''.join(val).encode('utf-8')
    for it in ray_vals:
        if en_val == it:
            break;
        i += 1

    return i
#####################################################################
def _map_car_vals(val):
    i = 0
    en_val = u''.join(val).encode('utf-8')
    for it in car_vals:
        if en_val == it:
            break;
        i += 1

    return i
#####################################################################
def _map_sick_vals(val):
    i = 0
    en_val = u''.join(val).encode('utf-8')
    for it in sick_vals:
        if en_val == it:
            break;
        i += 1

    return i
#####################################################################
# @c_id - cloth index, check @cloth_vals
def _create_train_data(r_id, s_id, c_id, car_id, obj, wid):
    fmt = '/tmp/a22301/%d_%d_%d_%d_%s.txt' %(r_id, s_id, c_id, car_id, wid)

#body_txt = '天1高%s 天1低%s 天1差%s 天2高%s 天2低%s 天2差%s 况1%s 况2%s 风1%s-%s 风2%s-%s' \
    body_txt = '天1高%s 天1低%s 天1差%s 天2高%s 天2低%s 天2差%s 况1%s 况2%s 风1%s-%s 风2%s-%s' \
               %(obj.temp_h1, obj.temp_l1, obj.delta1,
                 obj.temp_h2, obj.temp_l2, obj.delta2,
                 u''.join(obj.overal_1).encode('utf-8'), u''.join(obj.overal_2).encode('utf-8'),
                 u''.join(obj.wind_dir_1).encode('utf-8'), u''.join(obj.wind_frc_1).encode('utf-8'),
                 u''.join(obj.wind_dir_2).encode('utf-8'), u''.join(obj.wind_frc_2).encode('utf-8'))

    fd = open(fmt, 'w')
    try:
        fd.write(body_txt)
    except:
           print '%s || %s' %(sys.exc_info()[0], sys.exc_info()[1])
    finally:
        fd.close()

    return 0

#####################################################################
def _json_action(fn):
    fmt = 'data/%s' %(fn)
    print 'opening ... %s ...\n' %(fmt)
    zs_cloth = ''

    fd = open(fmt, 'r')
    try:
        all_txt = fd.read()
        print 'get all text, they are:\n'
        js_data = json.loads(all_txt)
#print json.dumps(js_data['zs'], ensure_ascii=False)
        zs = js_data['zs']
        zs_cloth = zs['z3']['v']
        cloth_index = _map_cloth_vals(zs_cloth)
        car_index = _map_car_vals(zs['z4']['v'])
        sick_index = _map_sick_vals(zs['z2']['v'])
        ray_index = _map_ray_vals(zs['z1']['v'])

        temp_high1 = js_data['fc']['a1']['d3']
        temp_low1 = js_data['fc']['a1']['d7']
        delta_temp1 = int(temp_high1) - int(temp_low1)


        temp_high2 = js_data['fc']['a2']['d3']
        temp_low2 = js_data['fc']['a2']['d7']
        delta_temp2 = int(temp_high2) - int(temp_low2)

        temp_high3 = js_data['fc']['a3']['d3']
        temp_low3 = js_data['fc']['a3']['d7']
        delta_temp3 = int(temp_high3) - int(temp_low3)

        obj = TempObject()
        obj.delta1 = _map_delta_vals(delta_temp1)
        obj.delta2 = _map_delta_vals(delta_temp2)
        obj.delta3 = _map_delta_vals(delta_temp3)
        obj.temp_h1 = _map_temp(int(temp_high1))
        obj.temp_l1 = _map_temp(int(temp_low1))
        obj.temp_h2 = _map_temp(int(temp_high2))
        obj.temp_l2 = _map_temp(int(temp_low2))
        obj.overal_1 = js_data['fc']['a1']['d2']
        obj.overal_2 = js_data['fc']['a2']['d2']

        obj.wind_dir_1 = js_data['fc']['a1']['d4']
        obj.wind_frc_1 = js_data['fc']['a1']['d5']
        obj.wind_dir_2 = js_data['fc']['a2']['d4']
        obj.wind_frc_2 = js_data['fc']['a2']['d5']
        obj.wind_dir_3 = js_data['fc']['a3']['d4']
        obj.wind_frc_3 = js_data['fc']['a3']['d5']

        print '%s - %s delta = %d' %(temp_high1, temp_low1, delta_temp1)

        _create_train_data(ray_index, sick_index, cloth_index,car_index, obj, fn)
    except:
           print '%s || %s' %(sys.exc_info()[0], sys.exc_info()[1])
    finally:
        fd.close()

    return 0

#####################################################################
def _simple_test(datadir):
    print 'will traverse the %s' %(datadir)
    for it in os.walk(datadir):
#print '--> %s\n' %(it[2])
        for fn in it[2]:
            _json_action(fn)

    return 0

#####################################################################
# try loading all traning data under 'data/' dir
def load_training_data(datadir):
    # ZS including:
    # 1 - ray (Zhi Wai Xian)
    #     0 - 最弱
    #     1 - 最弱
    # 2 - sick (Gan Mao)
    #     1 - 易发 
    #     2 - 极易发
    # 3 - cloth
    #     0 - 最弱
    #     1 - 寒冷
    # 4 - car 
    #     0 - 最弱
    #     1 - 易发 

    zs_idx = []
    cat_content = []

    for it in os.walk(datadir):
        print 'one step traverse'
        # FIXME - the it[2] store all list file...
        (cat_idx, cat_content) = _read_data_content(datadir, it[2])


    # try predict a new text benchmark checking paragraph
    for it in os.walk('compare_data/'):
        # FIXME - the it[2] store all list file...
        (c_idx, c_content) = _read_data_content('compare_data/', it[2])
    print 'the len of idx=%d' %(len(c_idx))
    print 'the len of content=%d' %(len(c_content))

    # NOTE - obviously , SVM is more accurate than Bayes
    print 'the auto-label for Bayes...'
    predict_with_bayes(cat_content, cat_idx, c_content, c_idx)

    print 'the auto-label for SVM...'
    predict_with_svm(cat_content, cat_idx, c_content, c_idx)

    return 0

#
# Main Entry Point:
#load_training_data('data/')
_simple_test('data')
#quick_insert_compare_data()
