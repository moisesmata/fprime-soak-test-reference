// ======================================================================
// \title  SensorDataProducerTester.cpp
// \author moisesmata
// \brief  cpp file for SensorDataProducer component test harness implementation class
//
// Follows the data-product testing approach described at
// https://fprime.jpl.nasa.gov/latest/docs/how-to/data-products/#testing
// ======================================================================

#include "SensorDataProducerTester.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

SensorDataProducerTester::SensorDataProducerTester()
    : SensorDataProducerGTestBase("SensorDataProducerTester", MAX_HISTORY_SIZE),
      component("SensorDataProducer"),
      m_getStatus(Fw::Success::SUCCESS) {
    this->initComponents();
    this->connectPorts();
}

SensorDataProducerTester::~SensorDataProducerTester() {}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

void SensorDataProducerTester::pushBmp(F32 pressure, F32 temperature) {
    Bmp280::Bmp280Data bmp;
    bmp.set_pressure(pressure);
    bmp.set_temperature(temperature);
    bmp.set_altitude(0.0f);
    this->invoke_to_bmpDataIn(0, bmp);
}

void SensorDataProducerTester::pushImu(F32 temperature) {
    MpuImu::ImuData imu;
    imu.set_temperature(temperature);
    imu.set_acceleration(FprimeSensors::GeometricVector3(1.0f, 2.0f, 3.0f));
    imu.set_rotation(FprimeSensors::GeometricVector3(4.0f, 5.0f, 6.0f));
    this->invoke_to_imuDataIn(0, imu);
}

Fw::Success::T SensorDataProducerTester::productGet_handler(FwDpIdType id, FwSizeType dataSize, Fw::Buffer& buffer) {
    this->pushProductGetEntry(id, dataSize);
    if (Fw::Success::SUCCESS == this->m_getStatus) {
        FW_ASSERT(dataSize <= sizeof(this->m_dpBuffer), static_cast<FwAssertArgType>(dataSize));
        buffer.set(this->m_dpBuffer, dataSize);
    }
    return this->m_getStatus;
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void SensorDataProducerTester::testBootStoppedBuffers() {
    this->pushBmp(101000.0f, 25.0f);

    ASSERT_PRODUCT_GET_SIZE(0);
    ASSERT_PRODUCT_SEND_SIZE(0);
    ASSERT_TLM_DpRecords(0, 1);
    ASSERT_EVENTS_DpStarted_SIZE(0);
}

void SensorDataProducerTester::testStoppedRingDropsOldest() {
    const U32 extraRecords = 3;
    for (U32 i = 0; i < SensorDataProducer::RECORDS_PER_CONTAINER + extraRecords; i++) {
        this->pushBmp(101000.0f + static_cast<F32>(i), 25.0f);
    }

    ASSERT_PRODUCT_GET_SIZE(0);
    ASSERT_PRODUCT_SEND_SIZE(0);
    ASSERT_TLM_DpRecords(this->tlmHistory_DpRecords->size() - 1, SensorDataProducer::RECORDS_PER_CONTAINER);
    ASSERT_TLM_DroppedRecords_SIZE(extraRecords);
    ASSERT_TLM_DroppedRecords(extraRecords - 1, extraRecords);
}

void SensorDataProducerTester::testStartFlushesFullRing() {
    this->sendCmd_START(0, 42);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, SensorDataProducerComponentBase::OPCODE_START, 42, Fw::CmdResponse::OK);
    ASSERT_TLM_WritingEnabled(0, true);
    ASSERT_EVENTS_WritingStarted_SIZE(1);

    for (U32 i = 0; i < SensorDataProducer::RECORDS_PER_CONTAINER; i++) {
        if ((i % 2) == 0) {
            this->pushBmp(101000.0f + static_cast<F32>(i), 25.0f);
        } else {
            this->pushImu(30.0f + static_cast<F32>(i));
        }
    }

    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_PRODUCT_SEND_SIZE(1);
    ASSERT_EVENTS_DpComplete_SIZE(1);
    ASSERT_EVENTS_DpComplete(0, SensorDataProducer::RECORDS_PER_CONTAINER);
    ASSERT_TLM_DpRecords(this->tlmHistory_DpRecords->size() - 1, 0);
}

void SensorDataProducerTester::testStopHaltsWriting() {
    this->sendCmd_START(0, 1);
    this->sendCmd_STOP(0, 2);
    ASSERT_CMD_RESPONSE_SIZE(2);
    ASSERT_CMD_RESPONSE(1, SensorDataProducerComponentBase::OPCODE_STOP, 2, Fw::CmdResponse::OK);
    ASSERT_TLM_WritingEnabled(1, false);
    ASSERT_EVENTS_WritingStopped_SIZE(1);

    for (U32 i = 0; i < SensorDataProducer::RECORDS_PER_CONTAINER; i++) {
        this->pushImu(30.0f + static_cast<F32>(i));
    }
    ASSERT_PRODUCT_GET_SIZE(0);
    ASSERT_PRODUCT_SEND_SIZE(0);
    ASSERT_TLM_DpRecords(this->tlmHistory_DpRecords->size() - 1, SensorDataProducer::RECORDS_PER_CONTAINER);
}

void SensorDataProducerTester::testAllocationFailure() {
    this->sendCmd_START(0, 0);
    this->m_getStatus = Fw::Success::FAILURE;
    for (U32 i = 0; i < SensorDataProducer::RECORDS_PER_CONTAINER; i++) {
        this->pushBmp(101000.0f, 25.0f);
    }

    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_PRODUCT_SEND_SIZE(0);
    ASSERT_EVENTS_DpMemoryFail_SIZE(1);
    ASSERT_TLM_DpRecords(this->tlmHistory_DpRecords->size() - 1, SensorDataProducer::RECORDS_PER_CONTAINER);
}

}  // namespace Components
