// ======================================================================
// \title  SensorDataAppTester.cpp
// \author moisesmata
// \brief  cpp file for SensorDataApp component test harness implementation class
//
// Follows the data-product testing approach described at
// https://fprime.jpl.nasa.gov/latest/docs/how-to/data-products/#testing
// ======================================================================

#include "SensorDataAppTester.hpp"

namespace SensorData {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

SensorDataAppTester::SensorDataAppTester()
    : SensorDataAppGTestBase("SensorDataAppTester", MAX_HISTORY_SIZE),
      component("SensorDataApp"),
      m_getStatus(Fw::Success::SUCCESS) {
    this->initComponents();
    this->connectPorts();
}

SensorDataAppTester::~SensorDataAppTester() {}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

void SensorDataAppTester::pushBothSensors(F32 pressure, F32 bmpTemp, F32 imuTemp) {
    Bmp280::Bmp280Data bmp;
    bmp.set_pressure(pressure);
    bmp.set_temperature(bmpTemp);
    bmp.set_altitude(0.0f);
    this->invoke_to_bmpDataIn(0, bmp);

    MpuImu::ImuData imu;
    imu.set_temperature(imuTemp);
    imu.set_acceleration(FprimeSensors::GeometricVector3(1.0f, 2.0f, 3.0f));
    imu.set_rotation(FprimeSensors::GeometricVector3(4.0f, 5.0f, 6.0f));
    this->invoke_to_imuDataIn(0, imu);
}

Fw::Success::T SensorDataAppTester::productGet_handler(FwDpIdType id, FwSizeType dataSize, Fw::Buffer& buffer) {
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

void SensorDataAppTester::testNoDataProductUntilBothSensors() {
    // A BMP reading alone must not produce a fused record or request a buffer
    Bmp280::Bmp280Data bmp;
    bmp.set_pressure(101000.0f);
    bmp.set_temperature(25.0f);
    bmp.set_altitude(0.0f);
    this->invoke_to_bmpDataIn(0, bmp);

    ASSERT_PRODUCT_GET_SIZE(0);
    ASSERT_TLM_FusedData_SIZE(0);

    // Once the IMU reading arrives, a fused record is produced and a container opened
    MpuImu::ImuData imu;
    imu.set_temperature(30.0f);
    imu.set_acceleration(FprimeSensors::GeometricVector3(0.0f, 0.0f, 0.0f));
    imu.set_rotation(FprimeSensors::GeometricVector3(0.0f, 0.0f, 0.0f));
    this->invoke_to_imuDataIn(0, imu);

    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_TLM_FusedData_SIZE(1);
    ASSERT_TLM_DpRecords(0, 1);
    ASSERT_EVENTS_DpStarted_SIZE(1);
    // Not full yet, so nothing sent
    ASSERT_PRODUCT_SEND_SIZE(0);
}

void SensorDataAppTester::testContainerSendsWhenFull() {
    // Push exactly RECORDS_PER_CONTAINER fused samples
    for (U32 i = 0; i < SensorDataApp::RECORDS_PER_CONTAINER; i++) {
        this->pushBothSensors(101000.0f + static_cast<F32>(i), 25.0f, 30.0f);
    }

    // Records accumulate into a single container that is sent once full
    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_PRODUCT_SEND_SIZE(1);
    ASSERT_EVENTS_DpComplete_SIZE(1);
    ASSERT_EVENTS_DpComplete(0, SensorDataApp::RECORDS_PER_CONTAINER);

    // State resets after the send: the counter telemetry ends at 0
    ASSERT_TLM_DpRecords(this->tlmHistory_DpRecords->size() - 1, 0);
}

void SensorDataAppTester::testRunFlushesPartialContainer() {
    // One fused record opens a container but does not fill it
    this->pushBothSensors(101000.0f, 25.0f, 30.0f);
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

void SensorDataAppTester::testAllocationFailure() {
    // Simulate a failed buffer allocation
    this->m_getStatus = Fw::Success::FAILURE;
    this->pushBothSensors(101000.0f, 25.0f, 30.0f);

    // A buffer was requested, allocation failed, and nothing was sent
    ASSERT_PRODUCT_GET_SIZE(1);
    ASSERT_PRODUCT_SEND_SIZE(0);
    ASSERT_EVENTS_DpMemoryFail_SIZE(1);
}

}  // namespace SensorDataApp
