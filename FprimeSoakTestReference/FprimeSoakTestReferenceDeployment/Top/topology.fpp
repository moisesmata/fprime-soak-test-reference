module FprimeSoakTestReference {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

  enum Ports_RateGroups {
    rateGroup1
    rateGroup2
    rateGroup3
    rateGroup4
  }

  topology FprimeSoakTestReferenceDeployment {

  # ----------------------------------------------------------------------
  # Subtopology imports
  # ----------------------------------------------------------------------
    import CdhCore.Subtopology
    import ComCcsds.Subtopology
    import DataProducts.Subtopology
    import DpCompression.Subtopology
    import FileHandling.Subtopology
    import MpuImu.Subtopology
    import Bmp280.Subtopology

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance chronoTime
    instance rateGroup1
    instance rateGroup2
    instance rateGroup3
    instance rateGroup4
    instance rateGroupDriver
    instance systemResources
    instance timer
    instance comDriver
    instance cmdSeq
    instance sensorDataProducer

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------

    command connections instance CdhCore.cmdDisp
    event connections instance CdhCore.events
    telemetry connections instance CdhCore.tlmSend
    text event connections instance CdhCore.textLogger
    health connections instance CdhCore.$health
    param connections instance FileHandling.prmDb
    time connections instance chronoTime

  # ----------------------------------------------------------------------
  # Telemetry packets (only used when TlmPacketizer is used)
  # ----------------------------------------------------------------------

    include "FprimeSoakTestReferenceDeploymentPackets.fppi"

  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections ComCcsds_CdhCore {
      # Core events and telemetry to communication queue
      CdhCore.events.PktSend -> ComCcsds.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.EVENTS]
      CdhCore.tlmSend.PktSend -> ComCcsds.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.TELEMETRY]

      # Router to Command Dispatcher
      ComCcsds.fprimeRouter.commandOut -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus -> ComCcsds.fprimeRouter.cmdResponseIn
      
    }

    connections ComCcsds_FileHandling {
      # File Downlink to Communication Queue
      FileHandling.fileDownlink.bufferSendOut -> ComCcsds.comQueue.bufferQueueIn[ComCcsds.Ports_ComBufferQueue.FILE]
      ComCcsds.comQueue.bufferReturnOut[ComCcsds.Ports_ComBufferQueue.FILE] -> FileHandling.fileDownlink.bufferReturn

      # Router to File Uplink
      ComCcsds.fprimeRouter.fileOut -> FileHandling.fileUplink.bufferSendIn
      FileHandling.fileUplink.bufferSendOut -> ComCcsds.fprimeRouter.fileBufferReturnIn
    }

    connections Communications {
      # ComDriver buffer allocations
      comDriver.allocate      -> ComCcsds.commsBufferManager.bufferGetCallee
      comDriver.deallocate    -> ComCcsds.commsBufferManager.bufferSendIn
      
      # ComDriver <-> ComStub (Uplink)
      comDriver.$recv                     -> ComCcsds.comStub.drvReceiveIn
      ComCcsds.comStub.drvReceiveReturnOut -> comDriver.recvReturnIn
      
      # ComStub <-> ComDriver (Downlink)
      ComCcsds.comStub.drvSendOut      -> comDriver.$send
      comDriver.ready         -> ComCcsds.comStub.drvConnected
    }

    connections FileHandling_DataProducts {
      # Data Products to File Downlink
      DataProducts.dpCat.fileOut -> FileHandling.fileDownlink.SendFile
      FileHandling.fileDownlink.FileComplete -> DataProducts.dpCat.fileDone
    }

    connections RateGroups {
      # timer to drive rate group
      timer.CycleOut -> rateGroupDriver.CycleIn

      # Rate group 1 (1kHz): Command sequencer for fast command dispatch. Only
      # non-blocking work belongs here given the 1ms cycle budget.
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup1] -> rateGroup1.CycleIn
      rateGroup1.RateGroupMemberOut[0] -> cmdSeq.schedIn

      # Rate group 2 (10Hz): Slow sensor, telemetry packetization, and communications.
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup2] -> rateGroup2.CycleIn
      rateGroup2.RateGroupMemberOut[0] -> Bmp280.bmpManager.run
      rateGroup2.RateGroupMemberOut[1] -> FileHandling.fileDownlink.Run
      rateGroup2.RateGroupMemberOut[2] -> ComCcsds.comQueue.run
      rateGroup2.RateGroupMemberOut[3] -> ComCcsds.aggregator.timeout
      rateGroup2.RateGroupMemberOut[4] -> CdhCore.tlmSend.Run
      # Flush partially-filled DP containers; RECORDS_PER_CONTAINER handles normal
      # batching of the sensor stream (currently a no-op under the fill-only policy).
      rateGroup2.RateGroupMemberOut[5] -> sensorDataProducer.run

      # Rate group 3 (1Hz): Housekeeping and health monitoring
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup3] -> rateGroup3.CycleIn
      rateGroup3.RateGroupMemberOut[0] -> CdhCore.$health.Run
      rateGroup3.RateGroupMemberOut[1] -> systemResources.run
      rateGroup3.RateGroupMemberOut[2] -> ComCcsds.commsBufferManager.schedIn
      rateGroup3.RateGroupMemberOut[3] -> DataProducts.dpBufferManager.schedIn
      rateGroup3.RateGroupMemberOut[4] -> DataProducts.dpWriter.schedIn
      rateGroup3.RateGroupMemberOut[5] -> DataProducts.dpMgr.schedIn
      rateGroup3.RateGroupMemberOut[6] -> DpCompression.Subtopology.dpZLibBufferManagerSchedIn

      # Rate group 4 (100Hz): IMU sensor read. The blocking-I2C read runs here with
      # a 10ms cycle budget, isolated from the 1kHz command path.
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup4] -> rateGroup4.CycleIn
      rateGroup4.RateGroupMemberOut[0] -> MpuImu.imuManager.run
    }

    connections CdhCore_cmdSeq {
      # Command Sequencer
      cmdSeq.comCmdOut -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus -> cmdSeq.cmdResponseIn
    }

    connections FprimeSoakTestReferenceDeployment {
       # Sensor managers push readings into the application component
       Bmp280.bmpManager.bmpDataPush -> sensorDataProducer.bmpDataIn
       MpuImu.imuManager.imuDataPush -> sensorDataProducer.imuDataIn

       # Application component produces data products (synchronous buffer request)
       sensorDataProducer.productGetOut  -> DataProducts.Subtopology.productGetIn
       sensorDataProducer.productSendOut -> DataProducts.Subtopology.productSendIn

       # Processing bit 0 routes marked containers through zlib compression.
       DataProducts.dpWriter.procBufferSendOut[0] -> DpCompression.Subtopology.dpCompressProcIn
    }

  }

}
