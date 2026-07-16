// ======================================================================
// \title  SensorDataApp.hpp
// \author moisesmata
// \brief  hpp file for SensorDataApp component implementation class
// ======================================================================

#ifndef SensorDataApp_SensorDataApp_HPP
#define SensorDataApp_SensorDataApp_HPP

#include "FprimeSoakTestReference/Components/SensorDataApp/SensorDataAppComponentAc.hpp"

namespace Components {

class SensorDataApp final : public SensorDataAppComponentBase {
  public:
    //! Number of fused records accumulated before a container is closed and sent
    static constexpr U32 RECORDS_PER_CONTAINER = 10;

    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct SensorDataApp object
    SensorDataApp(const char* const compName);

    //! Destroy SensorDataApp object
    ~SensorDataApp();

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

    //! Serialize the current fused sample into the open container, opening one
    //! if necessary, and send the container once it is full
    void writeFusedRecord();

    //! Send the open container and reset the record counter
    void closeAndSendContainer();

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! Most recent BMP data
    Bmp280::Bmp280Data m_lastBmpData;
    //! Most recent IMU data
    MpuImu::ImuData m_lastImuData;
    //! Whether BMP data has been received since the last fused record
    bool m_hasBmpData;
    //! Whether IMU data has been received since the last fused record
    bool m_hasImuData;

    //! The data product container being filled
    DpContainer m_container;
    //! Whether a container is currently open for writing
    bool m_dpInProgress;
    //! Number of records written into the open container
    U32 m_records;
};

}  // namespace Components

#endif
