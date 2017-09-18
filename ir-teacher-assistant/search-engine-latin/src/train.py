from sklearn.externals import joblib
from sklearn import linear_model
from sklearn.ensemble import ExtraTreesClassifier



def loadData(trainName):
    X = []
    Y = []
    
    with open(trainName, 'rb') as dataFile:
        for line in dataFile:
            fields = line.split()
            Y.append(float(fields[-1]))
            X.append([float(f) for f in fields[0:-1]])
    
    return X,Y


def train(trainName, modelName):
    logistic = ExtraTreesClassifier(n_estimators=40, verbose=2, random_state=0, max_features='auto',  n_jobs=-1) #linear_model.LogisticRegression()
    X, Y = loadData(trainName)
    model = logistic.fit(X, Y)
    joblib.dump(model, modelName, compress=9)
    
    
def classify(modelName, testName, predcitionName):
    model = joblib.load(modelName)
    X,Y = loadData(testName)
    print X
    print Y
    probs = model.predict_proba(X)
    
    fprediction = open(predcitionName, 'w')
    
    for prob in probs:
        fprediction.write(str(prob[1]))
        fprediction.write('\n')
    
    fprediction.close()
    
def main():
    trainPath = '/home/itamar/workspacePython/tpRi/train.txt'
    testPath = '/home/itamar/workspacePython/tpRi/test.txt'
    modelName = '/home/itamar/workspacePython/tpRi/model.txt'
    predcitionName = '/home/itamar/workspacePython/tpRi/prediction.txt'
    
    train(trainPath, modelName)
    #classify(modelName, testPath, predcitionName)
    
    print 'END'

if __name__=="__main__":
    main()