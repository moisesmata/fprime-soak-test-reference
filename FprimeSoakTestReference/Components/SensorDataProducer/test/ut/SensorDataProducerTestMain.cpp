// ======================================================================
// \title  SensorDataProducerTestMain.cpp
// \author moisesmata
// \brief  cpp file for SensorDataProducer component test main function
// ======================================================================

#include "SensorDataProducerTester.hpp"

TEST(Nominal, InactiveDropsData) {
    Components::SensorDataProducerTester tester;
    tester.testInactiveDropsData();
}

TEST(Nominal, BmpReadingWritesRecord) {
    Components::SensorDataProducerTester tester;
    tester.testBmpReadingWritesRecord();
}

TEST(Nominal, ImuReadingWritesRecord) {
    Components::SensorDataProducerTester tester;
    tester.testImuReadingWritesRecord();
}

TEST(Nominal, ContainerSendsWhenFull) {
    Components::SensorDataProducerTester tester;
    tester.testContainerSendsWhenFull();
}

TEST(Nominal, StopSendsPartialContainer) {
    Components::SensorDataProducerTester tester;
    tester.testStopSendsPartialContainer();
}

TEST(OffNominal, AllocationFailure) {
    Components::SensorDataProducerTester tester;
    tester.testAllocationFailure();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
