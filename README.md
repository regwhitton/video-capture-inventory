# Video Capture Inventory

![Build](https://github.com/regwhitton/video-capture-inventory/workflows/Build/badge.svg)

Video Capture Inventory provides a Java API and native shared libraries to give an inventory of
the video capture devices attached to the local machine and the frame sizes supported by each
device.

The supported platforms are:
* 64bit Windows

Not a long list, but it can only grow.

## OpenCV and the missing device information

OpenCV is a great toolkit for manipulating video.  Among many other things it allows the developer
to open any VideoCapture device, set the frame size to one supported by the device and grab frames.

You might do it like this:

    VideoCapture videoCapture = new VideoCapture();
    int camera = 0;
    if (videoCapture.open(camera)) {
        Mat mat = new Mat();

        videoCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
        videoCapture.set(CAP_PROP_FRAME_HEIGHT, 1080);

        videoCapture.read(mat);

        ... do something with the image
    }

Unfortunately, OpenCV provides no mechanism for finding out which devices or frame sizes are available,
and there doesn't seem to be another way to do this from Java.
Video Capture Inventory aims to provide this extra information.

You might use it like this:

    VideoCaptureInventory vci = VideoCaptureInventory.get();
    for(Device d : vci.devices){
        System.out.println("Camera: " + d.name);
        for(Format f : d.formats){
            System.out.println("  " + f.width + " x " + f.height);
        }
    }

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

You need a Github account and a personal access token to build against Github's Maven Repo (yes even if the project is public).
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

Alternatively, you can go to the packages tab on Github, download the jars, rename then and install them into your local repo.

## You might need to know

Currently Video Capture Inventory:

* Ignores scanners and still image devices.
* Ignores duplicate frame sizes for different colour depths.

## Example Maven Projects

* [Simple](./examples/simple) - list the devices and frame size to the console.
* [OpenCV](./examples/opencv) - use OpenCV to grab an image from every device, at every frame size.

# Credits

Includes NativeUtils by Adam Heinrich.  See: https://github.com/adamheinrich/native-utils
