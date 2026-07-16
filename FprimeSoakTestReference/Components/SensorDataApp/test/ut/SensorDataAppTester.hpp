// ======================================================================
// \title  SensorDataAppTester.hpp
// \author moisesmata
// \brief  hpp file for SensorDataApp component test harness implementation class
// ======================================================================

#ifndef SensorDataApp_SensorDataAppTester_HPP
#define SensorDataApp_SensorDataAppTester_HPP

#include "FprimeSoakTestReference/Components/SensorDataApp/SensorDataApp.hpp"
#include "FprimeSoakTestReference/Components/SensorDataApp/SensorDataAppGTestBase.hpp"

namespace SensorData {

class SensorDataAppTester : public SensorDataAppGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 100;
    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;
    // Backing store size for a requested data product buffer
    static const FwSizeType DP_BUFFER_SIZE = 4096;

    //! Construct object SensorDataAppTester
    SensorDataAppTester();

    //! Destroy object SensorDataAppTester
    ~SensorDataAppTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! A container is only allocated after both sensors have reported
    void testNoDataProductUntilBothSensors();

    //! Records accumulate and the container is sent once full
    void testContainerSendsWhenFull();

    //! The run port flushes a partially filled container
    void testRunFlushesPartialContainer();

    //! Allocation failure is handled gracefully
    void testAllocationFailure();

  private:
    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

    //! Push one BMP reading and one IMU reading into the component
    void pushBothSensors(F32 pressure, F32 bmpTemp, F32 imuTemp);

    //! Handle a data product buffer request from the component under test.
    //! Returns m_getStatus so tests can simulate allocation failure.
    Fw::Success::T productGet_handler(FwDpIdType id,        //!< The container ID
                                      FwSizeType dataSize,  //!< The requested data size
                                      Fw::Buffer& buffer    //!< The buffer (output)
                                      ) override;

    // ----------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------

    //! The component under test
    SensorDataApp component;

    //! Backing store handed out for data product buffers
    U8 m_dpBuffer[DP_BUFFER_SIZE];

    //! Status returned by productGet_handler (drives failure simulation)
    Fw::Success::T m_getStatus;
};

}  // namespace SensorDataApp

#endif
