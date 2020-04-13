# Video Capture Inventory

![Build](https://github.com/regwhitton/video-capture-inventory/workflows/Build/badge.svg)

Video Capture Inventory provides a Java API and native shared libraries to give an inventory of
the video capture devices attached to the local machine and the frame sizes supported by each
device.

## OpenCV and the missing device information

OpenCV is a great toolkit for manipulating video.  Among many other things it allows the developer
to open any VideoCapture device, set the frame size to one supported by the device and grab frames.

You might do it like this:

    VideoCapture videoCapture = new VideoCapture();
    int camera = 0;
    if (videoCapture.open(camera)) {
        videoCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
        videoCapture.set(CAP_PROP_FRAME_HEIGHT, 1080);
    
        Mat mat = new Mat();
        videoCapture.read(mat);
    
        ... do something with the Mat image
    }

Unfortunately, OpenCV does not provide a mechanism for finding out which devices or frame sizes are available.  Video Capture Inventory aims to provide this extra information.

You might use it like this:

    VideoCaptureInventory vci = VideoCaptureInventory.get();
    for (Device d : vci.getDevices()) {
        for (Format f : d.getFormats()) {
            if (f instanceof DiscreteFormat) {
                DiscreteFormat df = (DiscreteFormat) f;
                ...
            } else {
                StepwiseFormat sf = (StepwiseFormat) f;
                ...
            }
        }
    }

For examples of use see:

* [Simple](./examples/simple/src/main/java/com/github/regwhitton/videocaptureinventory/example/simple/SimpleExample.java) - list the devices and frame size to the console.
* [OpenCV](./examples/opencv/src/main/java/com/github/regwhitton/videocaptureinventory/example/opencv/OpenCvExample.java) - use OpenCV to grab an image from every device, at every frame size.

## Maven

Add the follow to your pom.xml to get the Java API and only the 64bit Windows native shared library:

    <dependencies>
        <dependency>
            <groupId>com.github.regwhitton</groupId>
            <artifactId>video-capture-inventory</artifactId>
            <version>0.1.1</version>
            <classifier/>
        </dependency>
        <dependency>
            <groupId>com.github.regwhitton</groupId>
            <artifactId>video-capture-inventory</artifactId>
            <version>0.1.1</version>
            <classifier>windows-x86_64</classifier>
        </dependency>
    </dependencies>
    ...
    <repositories>
        <repository>
            <id>github</id>
            <name>GitHub regwhitton Apache Maven Packages</name>
            <url>https://maven.pkg.github.com/regwhitton/video-capture-inventory</url>
        </repository>
    </repositories>

You need a Github account and a personal access token to build against Github's Maven Repo.
See [Creating a personal access token for the command line](https://help.github.com/en/github/authenticating-to-github/creating-a-personal-access-token-for-the-command-line)

Edit your ~/.m2/settings.xml and add a server entry.

    <settings ...>
        ...
        <servers>
            <server>
                <id>github</id>
                <username>your_github_username</username>
                <password>your_github_access_token</password>
            </server>
        </servers>
        ...
    </settings>

Alternatively, you can go to the packages tab on Github, download the jars and install them into your local repo.

### Maven Classifiers

| Platform Classifier | Target                                        | Tested On                    |
| ------------------- | --------------------------------------------- | ---------------------------- |
| windows-x86\_64     | Windows Vista and later (64bit Intel and AMD) | Dell XPS 13 9370 Windows 10  |
| linux-x86\_64       | Linux (64bit Intel and AMD)                   |                              |
| linux-armhf         | Raspberry Pi (32bit ARM)                      | Raspberry Pi 1 & 3           |

These classifiers are intended to align with those given to the OpenCV native shared libraries by [Javacpp-Presets](https://github.com/bytedeco/javacpp-presets).

Windows and Linux x86\_64 are available from the Github maven repo.  Raspberry Pi may need to be built manually. (See Raspberry Pi section below).

## You might need to know

Currently Video Capture Inventory:

* Ignores scanners and still image devices.
* Ignores duplicate frame sizes for different colour depths.

### Raspberry Pi

Cross-compiling for Raspberry Pi has proved difficult. However, the sources do build and work on a Pi.  Assuming you have a Java 8+ JDK and Maven 3.6+ installed, then you can build from the root source directory with:

    mvn install

Then in the example source directories:

    mvn verify

The Javacpp-Presets OpenCV linux-armhf library targets the Raspberry Pi, so is unlikely to work on other armhf builds of Linux as Debian.

March 2020: there are currently issues using OpenCV with USB cameras on the current version of Raspbian. See [OpenCV example](./examples/opencv/src/main/java/com/github/regwhitton/videocaptureinventory/example/opencv/OpenCvExample.java)

## License

[Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt)
