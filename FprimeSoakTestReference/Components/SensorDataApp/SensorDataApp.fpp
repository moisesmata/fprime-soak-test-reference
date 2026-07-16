# The module is deliberately named differently from the SensorDataApp component.
# If the module and component share a name, generated test harness code emits
# `SensorData::FusedSensorData` inside `namespace SensorData`, where the name would
# resolve to the component class instead of the module namespace and fail to compile.
module SensorData {

    @ Struct representing the fused sensor data product
    struct FusedSensorData {
        @ Pressure from BMP280 (Pa)
        pressure: F32
        @ Temperature from BMP280 (C)
        bmpTemperature: F32
        @ Temperature from IMU (C)
        imuTemperature: F32
        @ Acceleration from IMU (m/s^2)
        acceleration: FprimeSensors.GeometricVector3
        @ Angular rate from IMU (deg/s)
        rotation: FprimeSensors.GeometricVector3
    }

    @ Application component in the App-Manager-Driver pattern. Consumes data pushed
    @ from BmpManager and ImuManager, fuses it, and produces data products.
    passive component SensorDataApp {

        # ----------------------------------------------------------------------
        # Sensor data inputs (pushed from the sensor managers)
        # ----------------------------------------------------------------------

        @ Port for receiving Bmp280 sensor data
        sync input port bmpDataIn: Bmp280.Bmp280DataOut

        @ Port for receiving IMU sensor data
        sync input port imuDataIn: MpuImu.ImuDataOut

        @ Scheduling port used to manage the data product container lifecycle
        sync input port run: Svc.Sched

        # ----------------------------------------------------------------------
        # Data products
        # ----------------------------------------------------------------------

        @ Record holding one fused sensor sample
        product record FusedRecord: FusedSensorData id 0

        @ Container accumulating fused sensor records
        product container FusedDataContainer id 0 default priority 10

        # ----------------------------------------------------------------------
        # Telemetry
        # ----------------------------------------------------------------------

        @ Most recent fused sensor data
        telemetry FusedData: FusedSensorData

        @ Number of records written into the current data product container
        telemetry DpRecords: U32

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ A new data product container was opened
        event DpStarted \
            severity activity low \
            format "Opened new fused-data container"

        @ A data product container was filled and sent
        event DpComplete(records: U32) \
            severity activity low \
            format "Sent fused-data container with {} records"

        @ Failed to acquire a data product buffer
        event DpMemoryFail \
            severity warning high \
            format "Failed to acquire a data product buffer"

        @ Data product ports are not connected
        event DpsNotConnected \
            severity warning high \
            format "Data product ports are not connected" \
            throttle 5

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

        @ Data product get port (synchronous buffer allocation)
        product get port productGetOut

        @ Data product send port
        product send port productSendOut
    }
}
