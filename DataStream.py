import paho.mqtt.client as mqtt
import ssl
import struct       # for retrieving byte data from the device
import threading    # for running the mqtt loop in the background

BROKER = "3d0ef2c001394874af7fdfa932b5e994.s1.eu.hivemq.cloud"
PORT = 8883

class DataStream:
    def __init__(self):
        self.num_channels = None # metadata
        self.most_recent_data = None # an array to be returned when get_data() is called
        # used to ensure the same sample isn't retrieved twice by a caller.
        self.sample_num = 0 
        self.last_pulled_sample = -1
        # a lock to ensure that the writer and reader to not cause a conflict
        self.threadLock = threading.Lock() 

        self.mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.mqttc.tls_set(tls_version=ssl.PROTOCOL_TLS)
        self.mqttc.username_pw_set("Py_Sub", "Py_Sub")

        self.mqttc.on_connect = self.on_connect
        self.mqttc.on_message = self.on_message

        self.mqttc.connect(BROKER, PORT, 60)

        threading.Thread(target=self.mqttc.loop_forever, daemon=True).start()

    # the callback for when the client receives a CONNACK response from the server.
    def on_connect(self, client, userdata, flags, reason_code, properties):
        if __debug__: print(f"Connected with result code {reason_code}") # debugging
        # Subscribing in on_connect() means that if we lose the connection and
        # reconnect then subscriptions will be renewed.
        client.subscribe("EEG/size") # first get the number of channels
        client.subscribe("EEG/data") # then start receiving data
        
    # the callback for when a PUBLISH message is received from the server.
    def on_message(self, client, userdata, msg):
        global num_channels
        if (msg.topic == "EEG/data"):
            with self.threadLock:
                self.most_recent_data = struct.unpack(f"{self.num_channels}f", msg.payload)
            self.sample_num += 1
            if __debug__: print(msg.topic+" "+str(self.most_recent_data)) # debugging
        elif (msg.topic == "EEG/size"):
            self.num_channels = int(struct.unpack(f"i", msg.payload)[0])
            if __debug__: print(msg.topic+" "+str(self.num_channels)) # debugging

    
    def get_data(self):
        '''
        @returns: An array containing the most recently received value from the recording device.
                    if the sample has already been returned, this function will return None 
        '''
        # if this sample hasn't already been returned:
        if self.sample_num > self.last_pulled_sample:
            self.last_pulled_sample = self.sample_num
            with self.threadLock:
                return self.most_recent_data
        # else return nothing

def test_stream():
    stream = DataStream()

    while True:
        loop_array = stream.get_data()
        if loop_array is not None: # and not all(value == 0.0 for value in loop_array)
            print(f"{loop_array}\n")
        # else we don't print anything because there is no data.