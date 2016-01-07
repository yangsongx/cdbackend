#coding=utf-8
# Learn how to predict house price
import sys, os, re
import numpy
import jieba
from sklearn.datasets import load_boston
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.naive_bayes import MultinomialNB
from sklearn.linear_model import SGDClassifier
from sklearn.pipeline import Pipeline
from sklearn import metrics

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

# try loading all traning data under 'data/' dir
def load_training_data(datadir):
    cat_idx = []
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
boston = load_boston()

#quick_insert_compare_data()
