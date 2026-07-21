// ======================================================================
// \title  SensorDataProducerTester.hpp
// \author moisesmata
// \brief  hpp file for SensorDataProducer component test harness implementation class
// ======================================================================

#ifndef SensorDataProducer_SensorDataProducerTester_HPP
#define SensorDataProducer_SensorDataProducerTester_HPP

#include "FprimeSoakTestReference/Components/SensorDataProducer/SensorDataProducer.hpp"
#include "FprimeSoakTestReference/Components/SensorDataProducer/SensorDataProducerGTestBase.hpp"

namespace Components {

class SensorDataProducerTester : public SensorDataProducerGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 100;
    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;
    // Backing store size for a requested data product buffer
    static const FwSizeType DP_BUFFER_SIZE = 4096;

    //! Construct object SensorDataProducerTester
    SensorDataProducerTester();

    //! Destroy object SensorDataProducerTester
    ~SensorDataProducerTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! A BMP reading opens a container and writes a BMP record
    void testBmpReadingWritesRecord();

    //! An IMU reading opens a container and writes an IMU record
    void testImuReadingWritesRecord();

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

    //! Push one BMP reading into the component
    void pushBmp(F32 pressure, F32 temperature);

    //! Push one IMU reading into the component
    void pushImu(F32 temperature);

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
    SensorDataProducer component;

    //! Backing store handed out for data product buffers
    U8 m_dpBuffer[DP_BUFFER_SIZE];

    //! Status returned by productGet_handler (drives failure simulation)
    Fw::Success::T m_getStatus;
};

}  // namespace Components

#endif
