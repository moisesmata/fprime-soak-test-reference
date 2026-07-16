// ======================================================================
// \title  SensorDataApp.cpp
// \author moisesmata
// \brief  cpp file for SensorDataApp component implementation class
//
// Application component in the F Prime App-Manager-Driver pattern. It consumes
// sensor data pushed from BmpManager and ImuManager (each backed by its own
// bus driver), fuses the readings, and produces data products following
// https://fprime.jpl.nasa.gov/latest/docs/how-to/data-products/
// ======================================================================

#include "FprimeSoakTestReference/Components/SensorDataApp/SensorDataApp.hpp"

namespace SensorData {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SensorDataApp::SensorDataApp(const char* const compName)
    : SensorDataAppComponentBase(compName),
      m_hasBmpData(false),
      m_hasImuData(false),
      m_dpInProgress(false),
      m_records(0) {}

SensorDataApp::~SensorDataApp() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SensorDataApp::bmpDataIn_handler(FwIndexType portNum, const Bmp280::Bmp280Data& data) {
    this->m_lastBmpData = data;
    this->m_hasBmpData = true;
    this->writeFusedRecord();
}

void SensorDataApp::imuDataIn_handler(FwIndexType portNum, const MpuImu::ImuData& data) {
    this->m_lastImuData = data;
    this->m_hasImuData = true;
    this->writeFusedRecord();
}

void SensorDataApp::run_handler(FwIndexType portNum, U32 context) {
    // Flush any partially filled container on the schedule so that data is not
    // held indefinitely when the sensors produce samples slowly.
    if (this->m_dpInProgress && this->m_records > 0) {
        this->closeAndSendContainer();
    }
}

// ----------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------

bool SensorDataApp::openContainer() {
    // Data products must be available before we attempt to allocate a buffer
    if (!this->isConnected_productGetOut_OutputPort(0) || !this->isConnected_productSendOut_OutputPort(0)) {
        this->log_WARNING_HI_DpsNotConnected();
        return false;
    }

    // Size the buffer to hold RECORDS_PER_CONTAINER records. Each record carries
    // its serialized payload plus a record ID (sizeof(FwDpIdType)).
    const FwSizeType recordSize = FusedSensorData::SERIALIZED_SIZE + sizeof(FwDpIdType);
    const FwSizeType dpSize = RECORDS_PER_CONTAINER * recordSize;

    const Fw::Success status = this->dpGet_FusedDataContainer(dpSize, this->m_container);
    if (Fw::Success::FAILURE == status) {
        this->log_WARNING_HI_DpMemoryFail();
        return false;
    }

    this->m_dpInProgress = true;
    this->m_records = 0;
    this->log_ACTIVITY_LO_DpStarted();
    return true;
}

void SensorDataApp::writeFusedRecord() {
    // Only produce a fused record once we have a reading from both sensors
    if (!this->m_hasBmpData || !this->m_hasImuData) {
        return;
    }

    FusedSensorData fusedData;
    fusedData.set_pressure(this->m_lastBmpData.get_pressure());
    fusedData.set_bmpTemperature(this->m_lastBmpData.get_temperature());
    fusedData.set_imuTemperature(this->m_lastImuData.get_temperature());
    fusedData.set_acceleration(this->m_lastImuData.get_acceleration());
    fusedData.set_rotation(this->m_lastImuData.get_rotation());

    // Publish the latest fused sample as telemetry for real-time visibility
    this->tlmWrite_FusedData(fusedData);

    // Consume the readings so a stale sensor does not repeatedly contribute the
    // same value to subsequent fused records.
    this->m_hasBmpData = false;
    this->m_hasImuData = false;

    // Open a container lazily so we only request a buffer when there is data
    if (!this->m_dpInProgress) {
        if (!this->openContainer()) {
            return;
        }
    }

    const Fw::SerializeStatus stat = this->m_container.serializeRecord_FusedRecord(fusedData);
    if (Fw::FW_SERIALIZE_NO_ROOM_LEFT == stat) {
        // No room for this record: send what we have and start a fresh container
        this->closeAndSendContainer();
        if (this->openContainer()) {
            (void)this->m_container.serializeRecord_FusedRecord(fusedData);
            this->m_records++;
        }
    } else {
        FW_ASSERT(Fw::FW_SERIALIZE_OK == stat, static_cast<FwAssertArgType>(stat));
        this->m_records++;
    }

    this->tlmWrite_DpRecords(this->m_records);

    if (this->m_records >= RECORDS_PER_CONTAINER) {
        this->closeAndSendContainer();
    }
}

void SensorDataApp::closeAndSendContainer() {
    this->log_ACTIVITY_LO_DpComplete(this->m_records);
    this->dpSend(this->m_container);
    // The container is framework-owned after sending; reset our state
    this->m_dpInProgress = false;
    this->m_records = 0;
    this->tlmWrite_DpRecords(this->m_records);
}

}  // namespace SensorDataApp
