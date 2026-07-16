module CdhCore{

    # Using TlmPacketizer instead of TlmChan for efficient telemetry packetization
    instance tlmSend: Svc.TlmPacketizer base id CdhCoreConfig.BASE_ID + 0x06000 \
       queue size CdhCoreConfig.QueueSizes.tlmSend \
       stack size CdhCoreConfig.StackSizes.tlmSend \
       priority CdhCoreConfig.Priorities.tlmSend \
    {
       # Configure TlmPacketizer with deployment-specific packet list
       phase Fpp.ToCpp.Phases.configComponents """
       CdhCore::tlmSend.setPacketList(
           FprimeSoakTestReference::FprimeSoakTestReferenceDeployment_FprimeSoakTestReferenceDeploymentPacketsTlmPackets::packetList,
           FprimeSoakTestReference::FprimeSoakTestReferenceDeployment_FprimeSoakTestReferenceDeploymentPacketsTlmPackets::omittedChannels,
           1
       );
       """
    }
}
