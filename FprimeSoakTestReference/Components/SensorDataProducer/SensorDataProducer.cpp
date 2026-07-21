// ======================================================================
// \title  SensorDataProducer.cpp
// \author moisesmata
// \brief  cpp file for SensorDataProducer component implementation class
//
// Application component in the F Prime App-Manager-Driver pattern. It consumes
// sensor data pushed from BmpManager and ImuManager and produces data products following
// https://fprime.jpl.nasa.gov/latest/docs/how-to/data-products/
//
// Production is gated by the START/STOP commands. While started, BMP and IMU
// samples are serialized as records (BmpRecord and ImuRecord) into a shared
// container, which is sent as soon as it fills. STOP sends any partially
// filled container.
// ======================================================================

#include "FprimeSoakTestReference/Components/SensorDataProducer/SensorDataProducer.hpp"

namespace Components {

namespace {
//! Convert Fw::Time returned by getTime() into the displayable
//! Fw::TimeValue struct stored in data product records.
Fw::TimeValue toTimeValue(const Fw::Time& time) {
    return Fw::TimeValue(time.getTimeBase(), time.getContext(), time.getSeconds(), time.getUSeconds());
}
}  // namespace

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SensorDataProducer::SensorDataProducer(const char* const compName)
    : SensorDataProducerComponentBase(compName), m_active(false), m_dpInProgress(false), m_records(0) {}

SensorDataProducer::~SensorDataProducer() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SensorDataProducer::bmpDataIn_handler(FwIndexType portNum, const Bmp280::Bmp280Data& data) {
    // Production is command-gated: drop samples until START is received
    if (!this->m_active) {
        return;
    }

    // Open a container lazily so we only request a buffer when there is data
    if (!this->ensureContainerOpen()) {
        return;
    }

    // Build a timestamped BMP record.
    BmpSensorData record;
    record.set_timeTag(toTimeValue(this->getTime()));
    record.set_pressure(data.get_pressure());
    record.set_temperature(data.get_temperature());
    record.set_altitude(data.get_altitude());

    const Fw::SerializeStatus stat = this->m_container.serializeRecord_BmpRecord(record);
    if (Fw::FW_SERIALIZE_NO_ROOM_LEFT == stat) {
        // No room for this record: send what we have and start a fresh container
        this->closeAndSendContainer();
        if (this->ensureContainerOpen()) {
            (void)this->m_container.serializeRecord_BmpRecord(record);
            this->recordWritten();
        }
    } else {
        FW_ASSERT(Fw::FW_SERIALIZE_OK == stat, static_cast<FwAssertArgType>(stat));
        this->recordWritten();
    }
}

void SensorDataProducer::imuDataIn_handler(FwIndexType portNum, const MpuImu::ImuData& data) {
    // Production is command-gated: drop samples until START is received
    if (!this->m_active) {
        return;
    }

    // Open a container lazily so we only request a buffer when there is data
    if (!this->ensureContainerOpen()) {
        return;
    }

    // Build a timestamped IMU record. Records carry no implicit time, so we tag
    // each sample with the current time.
    ImuSensorData record;
    record.set_timeTag(toTimeValue(this->getTime()));
    record.set_temperature(data.get_temperature());
    record.set_acceleration(data.get_acceleration());
    record.set_rotation(data.get_rotation());

    const Fw::SerializeStatus stat = this->m_container.serializeRecord_ImuRecord(record);
    if (Fw::FW_SERIALIZE_NO_ROOM_LEFT == stat) {
        // No room for this record: send what we have and start a fresh container
        this->closeAndSendContainer();
        if (this->ensureContainerOpen()) {
            (void)this->m_container.serializeRecord_ImuRecord(record);
            this->recordWritten();
        }
    } else {
        FW_ASSERT(Fw::FW_SERIALIZE_OK == stat, static_cast<FwAssertArgType>(stat));
        this->recordWritten();
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void SensorDataProducer::START_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->m_active = true;
    this->tlmWrite_DpActive(true);
    this->log_ACTIVITY_HI_DpProductionStarted();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SensorDataProducer::STOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->m_active = false;
    // Send whatever has accumulated; an open container always holds at least
    // one record because containers are opened lazily on the first sample.
    if (this->m_dpInProgress) {
        this->closeAndSendContainer();
    }
    this->tlmWrite_DpActive(false);
    this->log_ACTIVITY_HI_DpProductionStopped();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ----------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------

bool SensorDataProducer::openContainer() {
    // Data products must be available before we attempt to allocate a buffer
    if (!this->isConnected_productGetOut_OutputPort(0) || !this->isConnected_productSendOut_OutputPort(0)) {
        this->log_WARNING_HI_DpsNotConnected();
        return false;
    }

    // Size the buffer to hold RECORDS_PER_CONTAINER records of whichever record
    // type is larger. Each record carries its serialized payload plus a record ID
    // (sizeof(FwDpIdType)); the record ID size is not automatically included in the
    // container data size because the number and types of records are up to us.
    const FwSizeType bmpRecordSize = BmpSensorData::SERIALIZED_SIZE + sizeof(FwDpIdType);
    const FwSizeType imuRecordSize = ImuSensorData::SERIALIZED_SIZE + sizeof(FwDpIdType);
    const FwSizeType maxRecordSize = (bmpRecordSize > imuRecordSize) ? bmpRecordSize : imuRecordSize;
    const FwSizeType dpSize = RECORDS_PER_CONTAINER * maxRecordSize;

    // Allocation may fail due to memory pressure; handle that case gracefully.
    const Fw::Success status = this->dpGet_SensorDataContainer(dpSize, this->m_container);
    if (Fw::Success::FAILURE == status) {
        this->log_WARNING_HI_DpMemoryFail();
        return false;
    }

    // Request zlib compression in DpWriter via DpCompression (port 0 / bit 0)
    this->m_container.setProcTypes(
        static_cast<Fw::DpCfg::ProcType::SerialType>(Fw::DpCfg::ProcType::PROC_TYPE_ZLIB_DEFLATE));
    this->m_dpInProgress = true;
    this->m_records = 0;
    this->log_ACTIVITY_LO_DpStarted();
    return true;
}

bool SensorDataProducer::ensureContainerOpen() {
    if (this->m_dpInProgress) {
        return true;
    }
    return this->openContainer();
}

void SensorDataProducer::recordWritten() {
    this->m_records++;
    this->tlmWrite_DpRecords(this->m_records);

    if (this->m_records >= RECORDS_PER_CONTAINER) {
        this->closeAndSendContainer();
    }
}

void SensorDataProducer::closeAndSendContainer() {
    this->log_ACTIVITY_LO_DpComplete(this->m_records);
    // The container is framework-owned after sending; we must not touch it again
    // until a new one is allocated.
    this->dpSend(this->m_container);
    this->m_dpInProgress = false;
    this->m_records = 0;
    this->tlmWrite_DpRecords(this->m_records);
}

}  // namespace Components
