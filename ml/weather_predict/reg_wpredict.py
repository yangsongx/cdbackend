#coding=utf-8

# reg_wpredict.py - weather predict via Regression Method
#

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
from sklearn import metrics, linear_model

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
def _json_action(fn, training_fd):
    fmt = 'data/%s' %(fn)
    print 'opening ... %s ...\n' %(fmt)

    fd = open(fmt, 'r')

    try:
        all_txt = fd.read()
        print 'get all text, they are:\n'
        js_data = json.loads(all_txt)
#print json.dumps(js_data['zs'], ensure_ascii=False)
        zs = js_data['zs']

        # get all Y values..
        cloth_index = _map_cloth_vals(zs['z3']['v'])
        car_index = _map_car_vals(zs['z4']['v'])
        sick_index = _map_sick_vals(zs['z2']['v'])
        ray_index = _map_ray_vals(zs['z1']['v'])

        # next will collect all X(input) parameters
        sd_val = js_data['ob']['r2']
        fx_val = js_data['ob']['r3']
        fl_val = js_data['ob']['r4']

        temp_high1 = js_data['fc']['a1']['d3']
        temp_low1 = js_data['fc']['a1']['d7']

        temp_high2 = js_data['fc']['a2']['d3']
        temp_low2 = js_data['fc']['a2']['d7']

        line_txt = '%s %s %s %s %s %s %s %s %s %s %s\n' \
                %(sd_val,fx_val,fl_val,temp_high1,temp_low1, temp_high2,temp_low2,cloth_index,car_index,sick_index,ray_index)

        training_fd.writelines(line_txt)

    except:
           print '%s || %s' %(sys.exc_info()[0], sys.exc_info()[1])
    finally:
        fd.close()

    return 0

def _predict_with_linear(ix,iy,tx,ty):
    regr = linear_model.LinearRegression()
    regr.fit(ix, iy)

    predict_Y = regr.predict(tx)
    print predict_Y.shape[0]

    print predict_Y
    for row in range(0, predict_Y.shape[0]):
            print '%d %d %d %d'  % (int(predict_Y[row][0]),int(predict_Y[row][1]),int(predict_Y[row][2]),int(predict_Y[row][3]))

    print 'real data:\n'
    print ty
    return 0

def _predict_with_ridge(ix,iy,tx,ty):
    clf = linear_model.Ridge(alpha=1.5)
    clf.fit(ix,iy)
    predict_Y = clf.predict(tx)
    print predict_Y.shape[0]

    print predict_Y
    for row in range(0, predict_Y.shape[0]):
            print '%d %d %d %d'  % (int(predict_Y[row][0]),int(predict_Y[row][1]),int(predict_Y[row][2]),int(predict_Y[row][3]))

    print 'real data:\n'
    print ty
    return 0

#####################################################################
# Reading traning data from @fn file
def _load_training_data(fn):
    fd = open(fn)
    data_count = 0
    line = fd.readline()
    while line:
        line = fd.readline()
        data_count += 1
    fd.close()

    print 'there are totally %d data' %(data_count)

    input_x = numpy.ndarray(shape=(data_count, 7), dtype=int)
    input_y = numpy.ndarray(shape=(data_count, 4), dtype=int)

    fd = open(fn)
    line = fd.readline()
    row = 0
    while line:
        fields = line.split() 
        print fields
        print 'the row is %d' %(row)
        for i in range(0, 7):
            input_x[row][i] = fields[i]
        j = 0 # for y index
        for i in range(7, 11):
            input_y[row][j] = fields[i]
            j += 1

        print 'finished mark\n'
        line = fd.readline()
        row += 1

    fd.close()

    print input_x.shape
    print input_y.shape

    input_X = input_x[:-20]
    input_Y = input_y[:-20]
    print 'after shrink, the traning size are:\n'
    print input_X.shape
    print input_Y.shape

    input_testx = input_x[-5:]
    input_testy = input_y[-5:]

    _predict_with_linear(input_X, input_Y, input_testx, input_testy)
    print 'Ridge Predict'
    _predict_with_ridge(input_X, input_Y, input_testx, input_testy)

    return 0

#####################################################################
# Reading raw weather html JSON data
# and stored data fields into the traning txt data
def _simple_test(datadir):
    print 'will traverse the %s' %(datadir)
    t_fd = open('/tmp/train.txt', 'a+')
    for it in os.walk(datadir):
#print '--> %s\n' %(it[2])
        for fn in it[2]:
            _json_action(fn, t_fd)
    t_fd.close()

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
_load_training_data('/tmp/train.txt')
#_simple_test('data')
#quick_insert_compare_data()
