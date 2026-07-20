# ======================================================================
# AcConstants.fpp (project override)
# F Prime configuration constants
# ======================================================================
#
# Project override of the framework default AcConstants.fpp. The only change
# from the framework default is RateGroupDriverRateGroupPorts, raised from 3 to
# 4 so the deployment can run four rate groups: 1kHz, 100Hz, 10Hz, and 1Hz.
# All other constants are copied verbatim from lib/fprime/default/config/AcConstants.fpp.

@ Number of rate group member output ports for ActiveRateGroup
constant ActiveRateGroupOutputPorts = 10

@ Number of rate group member output ports for PassiveRateGroup
constant PassiveRateGroupOutputPorts = 10

@ Used to drive rate groups
constant RateGroupDriverRateGroupPorts = 4

@ Used for command and registration ports
constant CmdDispatcherComponentCommandPorts = 30

@ Used for uplink/sequencer buffer/response ports
constant CmdDispatcherSequencePorts = 5

@ Used for dispatching sequences to command sequencers
constant SeqDispatcherSequencerPorts = 2

@ Used for sizing the command splitter input arrays
constant CmdSplitterPorts = CmdDispatcherSequencePorts

@ Number of static memory allocations
constant StaticMemoryAllocations = 4

@ Used to ping active components
constant HealthPingPorts = 25

@ Used for broadcasting completed file downlinks
constant FileDownCompletePorts = 1

@ Used for number of Fw::Com type ports supported by Svc::ComQueue
constant ComQueueComPorts = 2

@ Used for number of Fw::Buffer type ports supported by Svc::ComQueue
constant ComQueueBufferPorts = 1

@ Used for maximum number of connected buffer repeater consumers
constant BufferRepeaterOutputPorts = 10

@ Size of port array for DpManager
constant DpManagerNumPorts = 5

@ Size of processing port array for DpWriter
constant DpWriterNumProcPorts = 5

@ The size of a file name string
constant FileNameStringSize = 240

@ The size of an assert text string
constant FwAssertTextSize = 256

@ The size of a file name in an AssertFatalAdapter event (leading-truncation)
@ Note: File names in assertion failures are also truncated by
@ the constants FwAssertTextSize (in this file) and FW_LOG_STRING_MAX_SIZE.
@ Set smaller than FwAssertTextSize so there is room for the timestamp and
@ assertion arguments in the log message.
constant AssertFatalAdapterEventFileSize = FileNameStringSize

@ The maximum size in bytes for passing sequence arguments through CmdSeqIn ports
@ Note: This must fit within FW_CMD_ARG_BUFFER_MAX_SIZE along with command arguments
@ Evaluated from the framework defaults for this deployment. Configuration
@ override inputs must remain self-contained because consumers may import only
@ AcConstants.fpp.
constant SequenceArgumentsMaxSize = 255
