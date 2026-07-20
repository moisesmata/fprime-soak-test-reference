// ======================================================================
// \title  SensorDataProducerTestMain.cpp
// \author moisesmata
// \brief  cpp file for SensorDataProducer component test main function
// ======================================================================

#include "SensorDataProducerTester.hpp"

TEST(Nominal, BootStoppedBuffers) {
    Components::SensorDataProducerTester tester;
    tester.testBootStoppedBuffers();
}

TEST(Nominal, StoppedRingDropsOldest) {
    Components::SensorDataProducerTester tester;
    tester.testStoppedRingDropsOldest();
}

TEST(Nominal, StartFlushesFullRing) {
    Components::SensorDataProducerTester tester;
    tester.testStartFlushesFullRing();
}

TEST(Nominal, StopHaltsWriting) {
    Components::SensorDataProducerTester tester;
    tester.testStopHaltsWriting();
}

TEST(OffNominal, AllocationFailure) {
    Components::SensorDataProducerTester tester;
    tester.testAllocationFailure();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
