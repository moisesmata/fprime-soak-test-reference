// ======================================================================
// \title  SensorDataAppTestMain.cpp
// \author moisesmata
// \brief  cpp file for SensorDataApp component test main function
// ======================================================================

#include "SensorDataAppTester.hpp"

TEST(Nominal, NoDataProductUntilBothSensors) {
    SensorData::SensorDataAppTester tester;
    tester.testNoDataProductUntilBothSensors();
}

TEST(Nominal, ContainerSendsWhenFull) {
    SensorData::SensorDataAppTester tester;
    tester.testContainerSendsWhenFull();
}

TEST(Nominal, RunFlushesPartialContainer) {
    SensorData::SensorDataAppTester tester;
    tester.testRunFlushesPartialContainer();
}

TEST(OffNominal, AllocationFailure) {
    SensorData::SensorDataAppTester tester;
    tester.testAllocationFailure();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
