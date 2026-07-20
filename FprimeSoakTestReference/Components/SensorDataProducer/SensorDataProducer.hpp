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
    static constexpr U32 RECORDS_PER_CONTAINER = 50;

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
    // Command handler implementations
    // ----------------------------------------------------------------------

    //! Enable writing full data product containers
    void START_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    //! Disable writing and retain only the latest records
    void STOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    // ----------------------------------------------------------------------
    // Ring buffer types and helpers
    // ----------------------------------------------------------------------

    enum RecordType { RECORD_BMP, RECORD_IMU };

    struct RingEntry {
        RecordType type;
        BmpSensorData bmp;
        ImuSensorData imu;
    };

    //! Add a record, discarding the oldest record when the ring is full
    void pushEntry(const RingEntry& entry);

    //! Serialize and send all records in the full ring
    void flushRing();

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    RingEntry m_ring[RECORDS_PER_CONTAINER];
    U32 m_head;
    U32 m_count;
    bool m_writingEnabled;
    U32 m_droppedRecords;
};

}  // namespace Components

#endif
