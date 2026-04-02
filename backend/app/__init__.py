from flask import Flask 
from .config import Config
from .functions import DB 
from .mqtt import MQTT  

# Initialize DB
mongo = DB(Config)

# Initialize MQTT (pass DB into it)
Mqtt  = MQTT(mongo)

app = Flask(__name__)
app.config.from_object(Config) 

from app import views