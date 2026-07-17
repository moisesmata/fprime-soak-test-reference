// ======================================================================
// \title  SensorDataProducer.hpp
// \author moisesmata
// \brief  hpp file for SensorDataProducer component implementation class
// ======================================================================

#ifndef SensorDataProducer_SensorDataProducer_HPP
#define SensorDataProducer_SensorDataProducer_HPP

#include "FprimeSoakTestReference/Components/SensorDataProducer/SensorDataProducerComponentAc.hpp"

namespace Components {

class SensorDataProducer final : public SensorDataProducerComponentBase {
  public:
    //! Number of records (BMP or IMU) accumulated before a container is closed and sent
    static constexpr U32 RECORDS_PER_CONTAINER = 10;

    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct SensorDataProducer object
    SensorDataProducer(const char* const compName);

    //! Destroy SensorDataProducer object
    ~SensorDataProducer();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler for Bmp280 data pushed from BmpManager
    void bmpDataIn_handler(FwIndexType portNum, const Bmp280::Bmp280Data& data) override;

    //! Handler for IMU data pushed from ImuManager
    void imuDataIn_handler(FwIndexType portNum, const MpuImu::ImuData& data) override;

    //! Handler for the scheduling (rate group) input
    void run_handler(FwIndexType portNum, U32 context) override;

    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Acquire a data product buffer and open a new container
    //! \return true if a container was opened, false otherwise
    bool openContainer();

    //! Ensure a container is open for writing, opening one lazily if needed
    //! \return true if a container is available for writing, false otherwise
    bool ensureContainerOpen();

    //! Account for a record just written: bump the counter and send the
    //! container once it is full
    void recordWritten();

    //! Send the open container and reset the record counter
    void closeAndSendContainer();

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The data product container being filled
    DpContainer m_container;
    //! Whether a container is currently open for writing
    bool m_dpInProgress;
    //! Number of records written into the open container
    U32 m_records;
};

}  // namespace Components

#endif
