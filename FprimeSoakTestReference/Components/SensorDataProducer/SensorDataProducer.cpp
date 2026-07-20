// ======================================================================
// \title  SensorDataProducer.cpp
// \author moisesmata
// \brief  cpp file for SensorDataProducer component implementation class
//
// Application component in the F Prime App-Manager-Driver pattern. It consumes
// sensor data pushed from BmpManager and ImuManager and produces data products following
// https://fprime.jpl.nasa.gov/latest/docs/how-to/data-products/
//
// BMP and IMU samples are stored as separate record types (BmpRecord and
// ImuRecord) within a single shared container.
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
    : SensorDataProducerComponentBase(compName),
      m_head(0),
      m_count(0),
      m_writingEnabled(false),
      m_droppedRecords(0) {}

SensorDataProducer::~SensorDataProducer() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SensorDataProducer::bmpDataIn_handler(FwIndexType portNum, const Bmp280::Bmp280Data& data) {
    RingEntry entry;
    entry.type = RECORD_BMP;
    entry.bmp.set_timeTag(toTimeValue(this->getTime()));
    entry.bmp.set_pressure(data.get_pressure());
    entry.bmp.set_temperature(data.get_temperature());
    entry.bmp.set_altitude(data.get_altitude());
    this->pushEntry(entry);
}

void SensorDataProducer::imuDataIn_handler(FwIndexType portNum, const MpuImu::ImuData& data) {
    RingEntry entry;
    entry.type = RECORD_IMU;
    entry.imu.set_timeTag(toTimeValue(this->getTime()));
    entry.imu.set_temperature(data.get_temperature());
    entry.imu.set_acceleration(data.get_acceleration());
    entry.imu.set_rotation(data.get_rotation());
    this->pushEntry(entry);
}

void SensorDataProducer::run_handler(FwIndexType portNum, U32 context) {
    // Fill-only policy: scheduled ticks never flush partial rings.
}

// ----------------------------------------------------------------------
// Command handler implementations
// ----------------------------------------------------------------------

void SensorDataProducer::START_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->m_writingEnabled = true;
    this->tlmWrite_WritingEnabled(this->m_writingEnabled);
    this->log_ACTIVITY_HI_WritingStarted();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SensorDataProducer::STOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->m_writingEnabled = false;
    this->tlmWrite_WritingEnabled(this->m_writingEnabled);
    this->log_ACTIVITY_HI_WritingStopped();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ----------------------------------------------------------------------
// Ring buffer helpers
// ----------------------------------------------------------------------

void SensorDataProducer::pushEntry(const RingEntry& entry) {
    if (this->m_count == RECORDS_PER_CONTAINER) {
        this->m_ring[this->m_head] = entry;
        this->m_head = (this->m_head + 1U) % RECORDS_PER_CONTAINER;
        this->m_droppedRecords++;
        this->tlmWrite_DroppedRecords(this->m_droppedRecords);
    } else {
        const U32 tail = (this->m_head + this->m_count) % RECORDS_PER_CONTAINER;
        this->m_ring[tail] = entry;
        this->m_count++;
    }

    this->tlmWrite_DpRecords(this->m_count);
    if (this->m_writingEnabled && (this->m_count == RECORDS_PER_CONTAINER)) {
        this->flushRing();
    }
}

void SensorDataProducer::flushRing() {
    if (!this->isConnected_productGetOut_OutputPort(0) || !this->isConnected_productSendOut_OutputPort(0)) {
        this->log_WARNING_HI_DpsNotConnected();
        return;
    }

    const FwSizeType bmpRecordSize = BmpSensorData::SERIALIZED_SIZE + sizeof(FwDpIdType);
    const FwSizeType imuRecordSize = ImuSensorData::SERIALIZED_SIZE + sizeof(FwDpIdType);
    const FwSizeType maxRecordSize = (bmpRecordSize > imuRecordSize) ? bmpRecordSize : imuRecordSize;
    const FwSizeType dpSize = RECORDS_PER_CONTAINER * maxRecordSize;

    DpContainer container;
    const Fw::Success status = this->dpGet_SensorDataContainer(dpSize, container);
    if (Fw::Success::FAILURE == status) {
        this->log_WARNING_HI_DpMemoryFail();
        return;
    }

    // DpWriter processing port 0 is connected to the zlib compression processor.
    container.setProcTypes(static_cast<Fw::DpCfg::ProcType::SerialType>(1U));
    this->log_ACTIVITY_LO_DpStarted();

    for (U32 i = 0; i < this->m_count; i++) {
        const RingEntry& entry = this->m_ring[(this->m_head + i) % RECORDS_PER_CONTAINER];
        Fw::SerializeStatus stat = Fw::FW_SERIALIZE_OK;
        if (entry.type == RECORD_BMP) {
            stat = container.serializeRecord_BmpRecord(entry.bmp);
        } else {
            stat = container.serializeRecord_ImuRecord(entry.imu);
        }
        FW_ASSERT(Fw::FW_SERIALIZE_OK == stat, static_cast<FwAssertArgType>(stat));
    }

    this->log_ACTIVITY_LO_DpComplete(this->m_count);
    this->dpSend(container);
    this->m_head = 0;
    this->m_count = 0;
    this->tlmWrite_DpRecords(this->m_count);
}

}  // namespace Components
