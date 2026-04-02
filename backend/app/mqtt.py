import paho.mqtt.client as mqtt
from json import loads, dumps
from random import randint
from time import time

class MQTT:

    ID = f"IOT_B_{randint(1,1000000)}"

    sub_topics = [("620162191", 0)]

    def __init__(self, mongo):

        self.mongo = mongo

        self.client = mqtt.Client(client_id=self.ID)
        self.client.on_connect    = self.on_connect
        self.client.on_message    = self.on_message
        self.client.on_disconnect = self.on_disconnect

        # CONNECT TO LOCAL MOSQUITTO BROKER
        self.client.connect_async("localhost", 1883, 60)
        self.client.loop_start()

    def on_connect(self, client, userdata, flags, rc):
        print("MQTT Connected to local broker")
        client.subscribe(self.sub_topics)
        print(f"MQTT Subscribed to: {[t[0] for t in self.sub_topics]}")

    def on_disconnect(self, client, userdata, rc):
        if rc != 0:
            print("MQTT Unexpected disconnection.")

    def on_message(self, client, userdata, msg):
        try:
            topic   = msg.topic
            payload = msg.payload.decode()
            data    = loads(payload)

            # ADD TIMESTAMP
            data["timestamp"] = int(time())

            print(f"MQTT Received on [{topic}]: {data}")

            # STORE IN MONGODB
            self.mongo.insertSensorData(data)

            # PUBLISH TO FRONTEND TOPIC
            self.publish("620162191_pub", dumps(data))
            print(f"MQTT Published to 620162191_pub")

        except Exception as e:
            print(f"MQTT Error: {str(e)}")

    def publish(self, topic, payload):
        try:
            info = self.client.publish(topic, payload)
            info.wait_for_publish()
            return info.is_published()
        except Exception as e:
            print(f"MQTT Publish Error: {str(e)}")
            return False


