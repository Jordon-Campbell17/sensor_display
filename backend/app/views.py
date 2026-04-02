from app import app, mongo
from flask import jsonify, request
from time import time

#####################################
#       SENSOR DATA ROUTES          #
#####################################

# POST - Receive sensor data from ESP32
@app.route('/api/update', methods=['POST'])
def update():
    if request.method == "POST":
        data = request.get_json()
        if data:
            data['timestamp'] = int(time())
            result = mongo.insertSensorData(data)
            if result:
                return jsonify({"status": "complete", "data": "complete"})
    return jsonify({"status": "failed", "data": "failed"})


# GET - Return all sensor data from MongoDB
@app.route('/api/data', methods=['GET'])
def get_data():
    try:
        data = mongo.getAllData()
        return jsonify(data)
    except Exception as e:
        print("Error fetching data:", e)
        return jsonify({"status": "failed"})


#####################################
#        BASIC TEST ROUTE           #
#####################################

@app.route('/')
def home():
    return "Sensor Backend Running"


#####################################
#        ERROR HANDLING             #
#####################################

@app.errorhandler(404)
def not_found(error):
    return jsonify({"status": "not found"}), 404

@app.errorhandler(500)
def server_error(error):
    return jsonify({"status": "server error"}), 500

