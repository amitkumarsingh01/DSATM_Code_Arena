# Created by Amit Kumar Singh
from flask import Flask, render_template, jsonify
import requests

app = Flask(__name__)

ESP_IP = "http://192.168.3.169"

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/toggle_automated')
def toggle_automated():
    response = requests.get(f"{ESP_IP}/toggle_automated")
    return response.text

@app.route('/toggle_light')
def toggle_light():
    response = requests.get(f"{ESP_IP}/toggle_light")
    return response.text

@app.route('/toggle_dim')
def toggle_dim():
    response = requests.get(f"{ESP_IP}/toggle_dim")
    return response.text

@app.route('/get_data')
def get_data():
    response = requests.get(f"{ESP_IP}/get_data")
    return jsonify(response.json())

if __name__ == '__main__':
    app.run(debug=True)
