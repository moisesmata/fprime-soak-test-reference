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
        # Commands
        # ----------------------------------------------------------------------

        @ Start serializing sensor data into data product containers. Each
        @ container is sent as soon as it fills.
        sync command START

        @ Stop serializing sensor data. Any partially filled container is sent.
        sync command STOP

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

        @ Number of records written into the current data product container
        telemetry DpRecords: U32

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

        @ Data product ports are not connected
        event DpsNotConnected \
            severity warning high \
            format "Data product ports are not connected" \
            throttle 5

        @ Data product production was started by command
        event DpProductionStarted \
            severity activity high \
            format "Sensor data product production started"

        @ Data product production was stopped by command
        event DpProductionStopped \
            severity activity high \
            format "Sensor data product production stopped"

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
