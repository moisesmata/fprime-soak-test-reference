module Components {

    @ Struct representing one timestamped BMP280 sample stored as a data product record.
    struct BmpSensorData {
        @ Time the sample was received
        timeTag: Fw.TimeValue
        @ Pressure from BMP280 (Pa)
        pressure: F32
        @ Temperature from BMP280 (C)
        temperature: F32
        @ Altitude from BMP280 (m)
        altitude: F32
    }

    @ Struct representing one timestamped IMU sample stored as a data product record.
    struct ImuSensorData {
        @ Time the sample was received
        timeTag: Fw.TimeValue
        @ Temperature from IMU (C)
        temperature: F32
        @ Acceleration from IMU (m/s^2)
        acceleration: FprimeSensors.GeometricVector3
        @ Angular rate from IMU (deg/s)
        rotation: FprimeSensors.GeometricVector3
    }

    @ Start or stop sensor-data serialization
    enum SerializeAction {
        START @< Begin serializing sensor data into data products
        STOP  @< Stop serializing and flush any partial container
    }

    @ Application component in the App-Manager-Driver pattern. Consumes data pushed
    @ from BmpManager and ImuManager and produces data products 
    passive component SensorDataProducer {

        # ----------------------------------------------------------------------
        # Sensor data inputs (pushed from the sensor managers)
        # ----------------------------------------------------------------------

        @ Port for receiving Bmp280 sensor data
        sync input port bmpDataIn: Bmp280.Bmp280DataOut

        @ Port for receiving IMU sensor data
        sync input port imuDataIn: MpuImu.ImuDataOut

        # ----------------------------------------------------------------------
        # Mode interface
        # ----------------------------------------------------------------------

        @ Query the current spacecraft mode from ModeManager
        output port getCurrentMode: Svc.GetMode

        # ----------------------------------------------------------------------
        # Commands
        # ----------------------------------------------------------------------

        @ Start or stop serializing sensor data into data product containers.
        @ START is accepted only in EXPERIMENTATION mode. Entering SAFE while
        @ serializing automatically stops production.
        sync command SERIALIZE(
            @ Start or stop serialization
            op: SerializeAction
        )

        # ----------------------------------------------------------------------
        # State queries
        # ----------------------------------------------------------------------

        @ Report whether sensor data is currently being serialized. Returns
        @ SUCCESS if NOT serializing (safe to interrupt), FAILURE if actively
        @ serializing. Used by ModePolicy to gate EXPERIMENTATION -> IDLE.
        sync input port isSerializing: Fw.SuccessCondition

        # ----------------------------------------------------------------------
        # Data products
        # ----------------------------------------------------------------------

        @ Record holding one BMP280 sample
        product record BmpRecord: BmpSensorData id 0

        @ Record holding one IMU sample
        product record ImuRecord: ImuSensorData id 1

        @ Container accumulating BMP and IMU records
        product container SensorDataContainer id 0 default priority 10

        # ----------------------------------------------------------------------
        # Telemetry
        # ----------------------------------------------------------------------

        @ Whether sensor data is being serialized into data products
        telemetry DpActive: bool

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ A new data product container was opened
        event DpStarted \
            severity activity low \
            format "Opened new sensor data container"

        @ A data product container was filled and sent
        event DpComplete(records: U32) \
            severity activity low \
            format "Sent sensor data container with {} records"

        @ Failed to acquire a data product buffer
        event DpMemoryFail \
            severity warning high \
            format "Failed to acquire a data product buffer"

        @ Data product production was started by command
        event DpProductionStarted \
            severity activity high \
            format "Sensor data product production started"

        @ Data product production was stopped by command
        event DpProductionStopped \
            severity activity high \
            format "Sensor data product production stopped"

        @ SERIALIZE START rejected because the spacecraft is not in EXPERIMENTATION
        event SerializeRejectedWrongMode(
            current: Svc.Mode @< Mode at the time of the rejected start
        ) \
            severity warning low \
            format "Cannot start serialization in mode {} (requires EXPERIMENTATION)"

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Event port
        event port Log

        @ Text event port
        text event port LogText

        @ Command receive port
        command recv port CmdDisp

        @ Command registration port
        command reg port CmdReg

        @ Command response port
        command resp port CmdStatus

        @ Data product get port (synchronous buffer allocation)
        product get port productGetOut

        @ Data product send port
        product send port productSendOut
    }
}
