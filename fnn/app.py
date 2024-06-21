import os
import re
import numpy as np
import pandas as pd
from flask import Flask, request, render_template, redirect, url_for
from sklearn.model_selection import train_test_split, KFold
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score
import tensorflow as tf


def extract_features_from_code(code):
    features = {
        'num_lines': len(code.split('\n')),
        'num_functions': len(re.findall(r'\b[A-Za-z_]\w*\s*\(.*?\)\s*\{', code)),
        'num_include': code.count('#include'),
        'num_malicious_keywords': len(
            re.findall(r'\bshutdown\b|\bdelete\b|\bformat\b|\berase\b|\bkill\b|\bdangerous\b|\berror\b', code)),
        'num_suspicious_calls': len(
            re.findall(r'\bsystem\b|\bfopen\b|\bfwrite\b|\bexec\b|\bcreate\b|\bdelete\b', code)),
        'num_comments': len(re.findall(r'//|/\*|\*/', code)),
        'num_loops': len(re.findall(r'\bfor\b|\bwhile\b', code)),
        'num_conditionals': len(re.findall(r'\bif\b|\bswitch\b', code)),
        'num_memory_ops': len(re.findall(r'\bmalloc\b|\bfree\b|\bmemcpy\b|\bmemset\b', code)),
        'num_strings': len(re.findall(r'\".*?\"', code)),
        'length_of_code': len(code),
    }
    return features


def process_code_files(directory):
    data = []
    labels = []
    benign_count = 0
    malicious_count = 0

    for subdir, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.c'):
                file_path = os.path.join(subdir, file)
                with open(file_path, 'r') as f:
                    code = f.read()
                    features = extract_features_from_code(code)
                    label = 1 if 'malicious' in subdir else 0  # Label based on directory
                    data.append(features)
                    labels.append(label)
                    if label == 1:
                        malicious_count += 1
                    else:
                        benign_count += 1

    print(f'Benign samples: {benign_count}, Malicious samples: {malicious_count}')
    return data, labels


def load_and_preprocess_data(directory):
    data, labels = process_code_files(directory)
    df = pd.DataFrame(data)
    df['label'] = labels

    X = df.drop('label', axis=1).values
    y = df['label'].values

    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X)

    X_train, X_test, y_train, y_test = train_test_split(X_scaled, y, test_size=0.2, random_state=42)
    return X_train, X_test, y_train, y_test, scaler


def build_and_train_model(X_train, y_train):
    model = tf.keras.models.Sequential([
        tf.keras.layers.Dense(256, activation='relu', input_shape=(X_train.shape[1],)),
        tf.keras.layers.Dense(128, activation='relu'),
        tf.keras.layers.Dense(64, activation='relu'),
        tf.keras.layers.Dense(32, activation='relu'),
        tf.keras.layers.Dense(1, activation='sigmoid')
    ])

    model.compile(optimizer=tf.keras.optimizers.Adam(learning_rate=0.001), loss='binary_crossentropy',
                  metrics=['accuracy'])
    model.fit(X_train, y_train, epochs=50, validation_split=0.2)
    return model


def evaluate_model(model, X_test, y_test):
    predictions = (model.predict(X_test) > 0.5).astype(int)
    from sklearn.metrics import classification_report, confusion_matrix
    print(confusion_matrix(y_test, predictions))
    print(classification_report(y_test, predictions))
    loss, accuracy = model.evaluate(X_test, y_test)
    print(f'Test Accuracy: {accuracy}')


def cross_validate_model(X, y, n_splits=5):
    kf = KFold(n_splits=n_splits)
    accuracies = []
    precisions = []
    recalls = []
    f1s = []

    for train_index, test_index in kf.split(X):
        X_train, X_test = X[train_index], X[test_index]
        y_train, y_test = y[train_index], y[test_index]

        model = build_and_train_model(X_train, y_train)

        y_pred = (model.predict(X_test) > 0.5).astype(int)

        accuracies.append(accuracy_score(y_test, y_pred))
        precisions.append(precision_score(y_test, y_pred))
        recalls.append(recall_score(y_test, y_pred))
        f1s.append(f1_score(y_test, y_pred))

    print(f'Average Accuracy: {np.mean(accuracies)}')
    print(f'Average Precision: {np.mean(precisions)}')
    print(f'Average Recall: {np.mean(recalls)}')
    print(f'Average F1 Score: {np.mean(f1s)}')


app = Flask(__name__)

model = None
scaler = None


@app.route('/')
def home():
    return render_template('index.html')


@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return redirect(request.url)

    file = request.files['file']
    if file.filename == '':
        return redirect(request.url)

    if file:
        code = file.read().decode('utf-8')
        features = extract_features_from_code(code)
        X = pd.DataFrame([features]).values
        X_scaled = scaler.transform(X)

        prediction = model.predict(X_scaled)
        prediction = (prediction > 0.5).astype(int)

        return render_template('result.html', prediction=prediction[0][0])


if __name__ == '__main__':
    directory = 'data'
    X_train, X_test, y_train, y_test, scaler = load_and_preprocess_data(directory)

    model = build_and_train_model(X_train, y_train)

    model.save('model/malware_detection_model.h5')

    evaluate_model(model, X_test, y_test)

    data, labels = process_code_files(directory)
    df = pd.DataFrame(data)
    df['label'] = labels

    X = df.drop('label', axis=1).values
    y = df['label'].values

    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X)

    cross_validate_model(X_scaled, y)

    model = tf.keras.models.load_model('model/malware_detection_model.h5')

    app.run(debug=True)
