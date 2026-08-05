// ======================================================================
// \title  Main.cpp
// \brief main program for the F' application. Intended for CLI-based systems (Linux, macOS)
//
// ======================================================================
// Used to access topology functions
#include <FprimeSoakTestReference/FprimeSoakTestReferenceDeployment/Top/FprimeSoakTestReferenceDeploymentTopology.hpp>
// OSAL initialization
#include <Os/Os.hpp>
// Used for signal handling shutdown
#include <signal.h>
// Used for command line argument processing
#include <getopt.h>
// Used for atoi
#include <cstdlib>
// Used for logging to the console
#include <Fw/Logger/Logger.hpp>

/**
 * \brief print command line help message
 *
 * This will print a command line help message including the available command line arguments.
 *
 * @param app: name of application
 */
void print_usage(const char* app) {
    Fw::Logger::log(
        "Usage: ./%s [options]\n"
        "-a\tGDS IPv4 address (dotted-quad, e.g. 127.0.0.1)\n"
        "-p\tGDS UDP port (FSW sends here)\n"
        "-l\tLocal UDP bind port for FSW receive (default: GDS port + 1)\n"
        "-h\tHelp\n",
        app);
}

/**
 * \brief shutdown topology cycling on signal
 *
 * The reference topology allows for a simulated cycling of the rate groups. This simulated cycling needs to be stopped
 * in order for the program to shutdown. This is done via handling signals such that it is performed via Ctrl-C
 *
 * @param signum
 */
static void signalHandler(int signum) {
    FprimeSoakTestReference::stopRateGroups();
}

/**
 * \brief execute the program
 *
 * Communications use Drv::Udp against a GDS peer over Space Packet framing
 * (ComCcsds.SpacePacket). Pass -a/-p for the GDS address and UDP port.
 *
 * @param argc: argument count supplied to program
 * @param argv: argument values supplied to program
 * @return: 0 on success, something else on failure
 */
int main(int argc, char* argv[]) {
    I32 option = 0;
    char* hostname = nullptr;
    U16 port_number = 0;
    U16 local_port = 0;
    bool local_port_set = false;

    Os::init();

    // Loop while reading the getopt supplied options
    while ((option = getopt(argc, argv, "ha:p:l:")) != -1) {
        switch (option) {
            case 'a':
                hostname = optarg;
                break;
            case 'p':
                port_number = static_cast<U16>(atoi(optarg));
                break;
            case 'l':
                local_port = static_cast<U16>(atoi(optarg));
                local_port_set = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Object for communicating state to the topology
    FprimeSoakTestReference::TopologyState inputs;
    inputs.hostname = hostname;
    inputs.port = port_number;
    inputs.localPort = local_port_set ? local_port : static_cast<U16>(port_number == 0 ? 0 : port_number + 1);
    inputs.mpu.device = "/dev/i2c-1";
    inputs.bmp.device.device = 0; // SPI bus 0
    inputs.bmp.device.select = 0; // SPI chip select 0

    // Setup program shutdown via Ctrl-C
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    Fw::Logger::log("Hit Ctrl-C to quit\n");

    // Setup, cycle, and teardown topology
    FprimeSoakTestReference::setupTopology(inputs);
    FprimeSoakTestReference::startRateGroups(Fw::TimeInterval(0, 1000));  // Program loop cycling rate groups at 1KHz
    FprimeSoakTestReference::teardownTopology(inputs);
    Fw::Logger::log("Exiting...\n");
    return 0;
}
