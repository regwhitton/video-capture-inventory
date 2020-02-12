# OpenCV Example
`
Test Application that uses Video Capture Inventory with OpenCV (via JavaCPP).

Grabs a frame from every camera at every resolution and writes it to `target`.

Build and run the example on your machine using maven:

    mvn verify

Note: JavaCPP unpacks the OpenCV native shared libraries languages into `~/.javacpp` and leaves them there.
