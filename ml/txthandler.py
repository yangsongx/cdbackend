#!/usr/bin/python

import logging
import numpy
from sklearn.datasets import fetch_20newsgroups
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.naive_bayes import MultinomialNB
from sklearn.linear_model import SGDClassifier
from sklearn.pipeline import Pipeline

categories = ['alt.atheism', 'soc.religion.christian','comp.graphics', 'sci.med']


def do_job_in_one_line(data, target, testdata):
    text_clf = Pipeline(
            [
              ('vect', CountVectorizer()),
              ('tfidf', TfidfTransformer()),
              ('clf', MultinomialNB()),
            ])
    text_clf = text_clf.fit(data, target)
    mypredict = text_clf.predict(testdata.data)
    print 'finished the def'
    print numpy.mean(mypredict == testdata.target)

def do_job_in_svm(data, target, testdata):
    text_clf = Pipeline(
            [
              ('vect', CountVectorizer()),
              ('tfidf', TfidfTransformer()),
              ('clf', SGDClassifier(loss='hinge', penalty='l2',
                                    alpha=1e-3, n_iter=5, random_state=42)),
            ])
    text_clf = text_clf.fit(data, target)
    mypredict = text_clf.predict(testdata.data)
    print 'finished the SVM def'
    print numpy.mean(mypredict == testdata.target)

print 'Will loading 20 newsgroups, take quite many minutes...'
twenty_train = fetch_20newsgroups(subset='train',
        categories=categories, shuffle=True, random_state=42)
print 'data set loading finished[OK]'

# tokenize the text...
count_vect = CountVectorizer()
X_train_counts = count_vect.fit_transform(twenty_train.data)
tf_transformer = TfidfTransformer(use_idf=False).fit(X_train_counts)
X_train_tf = tf_transformer.transform(X_train_counts)

tfidf_transformer = TfidfTransformer()
X_train_tfidf = tfidf_transformer.fit_transform(X_train_counts)

clf = MultinomialNB().fit(X_train_tfidf, twenty_train.target)

docs_new = ['God is love', 'OpenGL on the GPU is fast']
X_new_counts = count_vect.transform(docs_new)
X_new_tfidf = tfidf_transformer.transform(X_new_counts)
predicted = clf.predict(X_new_tfidf)
for doc, category in zip(docs_new, predicted):
    print('%r => %s' % (doc, twenty_train.target_names[category]))

twenty_test = fetch_20newsgroups(subset='test',
        categories=categories, shuffle=True, random_state=42)
do_job_in_one_line(twenty_train.data, twenty_train.target, twenty_test)
do_job_in_svm(twenty_train.data, twenty_train.target, twenty_test)
