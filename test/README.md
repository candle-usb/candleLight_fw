# candleLight_gsusb tests #

The script on this directory performs a limited test for
device functionality. It is meant to be run with a device
under test (DUT) connected thought USB and JTAG/SWD, and
another CAN interface (AUX) used to send stimulus to the
DUT through the CAN bus. Therefore, DUT and AUX must both
* be visible in as system interfaces
* be connected ot the same CAN bus

The test is sligtly intrusive, because it temporarily
suspends the FW capability of sending frames to the host
through USB. This is done by building a special "test"
version of the FW which includes some extra code
instrumentation.

## Running the test ##

1. Prepare your HW setup:
    * DUT and AUX connected to the same CAN bus
	* DUT connected to the host by JTAG/SWD
	* GDB server (openocd, most likely) running

2. Start by building the special test version of the
   code (append `_test_fw` suffix to your usual target name):

        build$ make FYSETC_UCAN_test_fw

3. Launch the test; with default parameters, it will
   flash the device and take care of everything:

        build$ ../test/test.sh FYSETC_UCAN can1 can0
		Free buffers idle=62, after=62

4. For better coverage, launch the test in a loop using the shell.
   Consider using `timeout` as some error modes might make the script
   wait forever:

        build$ i=0; while true; do i=$((i+1)); echo -n "Iteration $i: "; timeout 20 ../test/test.sh -r FYSETC_UCAN can1 can0 || break; done
        Iteration 1: Free buffers idle=62, after=62
        Iteration 2: Free buffers idle=62, after=62
        Iteration 3: Free buffers idle=62, after=62
        Iteration 4: Free buffers idle=62, after=62
        Iteration 5: Free buffers idle=62, after=62
        Iteration 6: Free buffers idle=62, after=62
        Iteration 7: Free buffers idle=62, after=62
		[...]

## Caveats ##

* Checking the diff between the send and received frames fails
  sometimes. This is why this check is not done by default
  (it can be activated with `-d` parameter).
