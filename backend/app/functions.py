class DB:

    def __init__(self, Config):
        from pymongo import MongoClient
        from urllib import parse

        self.server   = Config.DB_SERVER
        self.port     = Config.DB_PORT
        self.username = parse.quote_plus(Config.DB_USERNAME)
        self.password = parse.quote_plus(Config.DB_PASSWORD)

        self.client = MongoClient(
            f'mongodb://{self.username}:{self.password}@{self.server}:{self.port}'
        )

        self.db = self.client["ELET2415"]
        self.collection = self.db["sensor_data"]

    # ✅ STORE SENSOR DATA
    def insertSensorData(self, data):
        try:
            self.collection.insert_one(data)
            return True
        except Exception as e:
            print("Insert Error:", e)
            return False

    # ✅ GET ALL DATA
    def getAllData(self):
        try:
            return list(self.collection.find({}, {"_id": 0}))
        except Exception as e:
            print("Fetch Error:", e)
            return []