#coding=utf-8
import sys, os, re
import numpy
from sklearn.datasets import fetch_20newsgroups
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.naive_bayes import MultinomialNB
from sklearn.linear_model import SGDClassifier
from sklearn.pipeline import Pipeline
from sklearn import metrics

def predict_with_bayes():
    # create the pipline for Classifier
    txt_clf = Pipeline(
            [
                ('vect', CountVectorizer()),
                ('tfidf', TfidfTransformer()),
                ('clf', MultinomialNB()),
            ])
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
        print '%d --> %s' %(t, 'sth')

    return (cat_idx, cat_content)

# try loading all traning data under 'data/' dir
def load_training_data(datadir):
    cat_idx = []
    cat_content = []

    for it in os.walk(datadir):
        print 'one step traverse'
        # FIXME - the it[2] store all list file...
        (cat_idx, cat_content) = _read_data_content(datadir, it[2])

    # next try learning it
    print 'prepare traning...'
    txt_clf = Pipeline(
            [
                ('vect', CountVectorizer()),
                ('tfidf', TfidfTransformer()),
                ('clf', MultinomialNB()),
            ])
    txt_clf = txt_clf.fit(cat_content, cat_idx)
    print 'finished traning...'

    # try predict a new text paragraph
    for it in os.walk('compare_data/'):
        # FIXME - the it[2] store all list file...
        (c_idx, c_content) = _read_data_content(datadir, it[2])
    predicted = txt_clf.predict(c_content)
    print predicted

    print numpy.mean(predicted == c_idx)

    print(metrics.classification_report(c_idx, predicted, c_idx))

    return 0

#
# Main Entry Point:
load_training_data('data/')
