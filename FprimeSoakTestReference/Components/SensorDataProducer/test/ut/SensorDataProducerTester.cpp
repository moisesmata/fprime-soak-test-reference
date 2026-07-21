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

void SensorDataProducerTester::testBmpReadingWritesRecord() {
    // A single BMP reading opens a container, writes one record, and publishes telemetry
    this->pushBmp(101000.0f, 25.0f);

    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_TLM_BmpData_SIZE(1);
    ASSERT_TLM_DpRecords(0, 1);
    ASSERT_EVENTS_DpStarted_SIZE(1);
    // Not full yet, so nothing sent
    ASSERT_PRODUCT_SEND_SIZE(0);
}

void SensorDataProducerTester::testImuReadingWritesRecord() {
    // A single IMU reading opens a container, writes one record, and publishes telemetry
    this->pushImu(30.0f);

    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_TLM_ImuData_SIZE(1);
    ASSERT_TLM_DpRecords(0, 1);
    ASSERT_EVENTS_DpStarted_SIZE(1);
    // Not full yet, so nothing sent
    ASSERT_PRODUCT_SEND_SIZE(0);
}

void SensorDataProducerTester::testContainerSendsWhenFull() {
    // Interleave BMP and IMU readings. Both record types share one container and
    // one counter, so RECORDS_PER_CONTAINER total records fill it regardless of type.
    for (U32 i = 0; i < SensorDataProducer::RECORDS_PER_CONTAINER; i++) {
        if ((i % 2) == 0) {
            this->pushBmp(101000.0f + static_cast<F32>(i), 25.0f);
        } else {
            this->pushImu(30.0f + static_cast<F32>(i));
        }
    }

    // Records accumulate into a single container that is sent once full
    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_PRODUCT_SEND_SIZE(1);
    ASSERT_EVENTS_DpComplete_SIZE(1);
    ASSERT_EVENTS_DpComplete(0, SensorDataProducer::RECORDS_PER_CONTAINER);

    // State resets after the send: the counter telemetry ends at 0
    ASSERT_TLM_DpRecords(this->tlmHistory_DpRecords->size() - 1, 0);
}

void SensorDataProducerTester::testRunFlushesPartialContainer() {
    // One BMP record opens a container but does not fill it
    this->pushBmp(101000.0f, 25.0f);
    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_PRODUCT_SEND_SIZE(0);
    ASSERT_TLM_DpRecords(0, 1);

    // The scheduled run flushes the partial container
    this->invoke_to_run(0, 0);
    ASSERT_PRODUCT_SEND_SIZE(1);
    ASSERT_EVENTS_DpComplete_SIZE(1);
    ASSERT_EVENTS_DpComplete(0, 1);

    // Counter resets to 0 after the flush
    ASSERT_TLM_DpRecords(this->tlmHistory_DpRecords->size() - 1, 0);

    // A subsequent run with no new data does nothing
    this->clearHistory();
    this->invoke_to_run(0, 0);
    ASSERT_PRODUCT_SEND_SIZE(0);
}

void SensorDataProducerTester::testAllocationFailure() {
    // Simulate a failed buffer allocation
    this->m_getStatus = Fw::Success::FAILURE;
    this->pushBmp(101000.0f, 25.0f);

    // A buffer was requested, allocation failed, and nothing was sent
    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_PRODUCT_SEND_SIZE(0);
    ASSERT_EVENTS_DpMemoryFail_SIZE(1);
}

}  // namespace Components
