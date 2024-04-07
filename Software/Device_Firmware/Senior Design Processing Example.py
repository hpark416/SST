## Data Processing Example ##
import numpy as np

## Setup ##
sensorShape = (4,4)
sensorData = np.full(sensorShape, np.random.normal(93.2, 0.62)) #Ground zero of body temp
sensorDataAccident = np.copy(sensorData)
sensorDataAccident[2:,:] = np.random.normal(98.2, 0.62)
heatOverTime = np.full((5, sensorShape[0], sensorShape[1]), sensorData) #nothing has changed for a while

accident = False #flag to simulate if an accident has occurred

## Process ##

#returns true if an accident occurred, false otherwise
def processHeat():
    if not(np.allclose(heatOverTime[0], heatOverTime[-1], rtol=0.001, atol=0.62)):
        return True
    else:
        return False

#example of how the sensor data would update
def updateSensors(accident):
    if (np.random.randint(0,10) == 0):
        accident = True
    
    if accident:
        heatOverTime[:-1] = heatOverTime[1:]; heatOverTime[-1] = sensorDataAccident
    

## Print ##
for i in range(20):
    updateSensors(accident)
    if processHeat():
        print("Alert! An accident occurred")
    else:
        print("Reading...")
