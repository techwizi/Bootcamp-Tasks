import paho.mqtt.client as mqtt

# MQTT settings
mqtt_server = "YOUR_RASPBERRY_PI_IP"
mqtt_port = 1883
mqtt_topic = "ultrasonic/data"

# Distance threshold to trigger LED
distance_threshold = 20  # Adjust this threshold as needed

# Callback when connected to the MQTT broker
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker with result code " + str(rc))
    client.subscribe(mqtt_topic)

# Callback when a message is received from the MQTT broker
def on_message(client, userdata, msg):
    print("Received message on topic " + msg.topic + ": " + str(msg.payload, 'utf-8'))
    
    try:
        distance = float(msg.payload.decode())
        if distance < distance_threshold:
            print("Distance is below the threshold. Turning on LED on NodeMCU.")
            # Send a control message to NodeMCU to turn on the LED
            client.publish("ultrasonic/ctrl", "on")
        else:
            print("Distance is above the threshold. Turning off LED on NodeMCU.")
            # Send a control message to NodeMCU to turn off the LED
            client.publish("ultrasonic/ctrl", "off")
    except ValueError:
        print("Invalid distance value received.")

# Create MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT broker
client.connect(mqtt_server, mqtt_port, 60)

# Loop to listen for messages
client.loop_forever()